#include "CalcClient.h"
#include "common/async_io/CalcLine.h"
#include "common/io/print.h"
#include "no-coroutines/04-async-io/async/AsyncIo.h"

#include <format>
#include <stdexcept>
#include <string>

#include <unistd.h>

namespace no_coroutines {

// Store the reactor and file descriptors.
CalcClient::CalcClient(Reactor &reactor, const int stdin_fd, const int write_fd, const int read_fd)
    : _reactor(reactor), _stdin_fd(stdin_fd), _write_fd(write_fd), _read_fd(read_fd) {}

// Acquire the WorkGuard and start the async stdin read loop.
void CalcClient::run() {
    _work_guard = std::make_unique<WorkGuard>(_reactor);
    io::print("[Client] Started\n");
    async_read_from_stdin();
}

// Post a non-blocking read that calls on_read_complete when a full line arrives.
void CalcClient::async_read_from_stdin() {
    io::print("[Client] Waiting for input...\n");
    post_read(
        _reactor, _stdin_fd, _write_buffer, sizeof(_write_buffer),
        stop_at_newline_or_full(),
        [this](const size_t bytes_read) {
            on_read_complete(bytes_read);
        });
}

// Forward the line to the server, or stop on EOF.
void CalcClient::on_read_complete(const size_t bytes_read) {
    if (bytes_read == 0) {
        // stdin reached EOF. Close the server pipe to signal no more input.
        io::print("[Client] EOF on stdin\n");
        close(_write_fd);
        io::print("[Client] Finished\n");
        _work_guard.reset();
        return;
    }

    log_input(_write_buffer, bytes_read);
    async_write_to_server(bytes_read);
}

// Print the raw expression read from stdin.
void CalcClient::log_input(const char *data, const size_t size) {
    io::print("[Client] Read from stdin: {}", std::string_view(data, size));
}

// Post a non-blocking write of the expression bytes to the server pipe.
void CalcClient::async_write_to_server(const size_t size) {
    post_write(_reactor, _write_fd, _write_buffer, size,
               [this, size](const size_t actual) { on_write_complete(size, actual); });
}

// Verify the write, pre-compute the expected response, and kick off the async response read.
void CalcClient::on_write_complete(const size_t expected, const size_t actual) {
    verify_write_complete(expected, actual);

    // Compute the expected result locally so we know how many bytes to read from the server.
    _expected_response = async_io::CalcLine::eval({_write_buffer, expected});
    async_read_response(_expected_response.size());
}

// Throw if fewer bytes were written to the server pipe than expected.
void CalcClient::verify_write_complete(const size_t expected, const size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format(
            "[Client] Failed to write all bytes to pipe_client_to_server! Expected {} bytes, wrote {} bytes",
            expected, actual));
    }
    io::print("[Client] Wrote {} bytes to pipe_client_to_server\n", actual);
}

// Post a non-blocking read of exactly size response bytes from the server pipe.
void CalcClient::async_read_response(const size_t size) {
    post_read(_reactor, _read_fd, _read_buffer, size,
              stop_at_exact_bytes(),
              [this, size](const size_t actual) { on_read_response_complete(size, actual); });
}

// Verify the response and loop back to read the next stdin line.
void CalcClient::on_read_response_complete(const size_t expected, const size_t actual) {
    verify_read_complete(expected, actual);
    verify_and_log_response({_read_buffer, actual}, _expected_response);
    async_read_from_stdin();
}

// Throw if fewer bytes were received from the server than expected.
void CalcClient::verify_read_complete(const size_t expected, const size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format(
            "[Client] Failed to read all bytes from pipe_server_to_client! Expected {} bytes, got {} bytes",
            expected, actual));
    }
}

// Throw if the response does not match the expected value; log success otherwise.
void CalcClient::verify_and_log_response(const std::string_view received, const std::string_view expected) {
    if (received != expected) {
        throw std::runtime_error("[Client] Response mismatch!");
    }
    io::print("[Client] Read from pipe_server_to_client: {}", received);
    io::print("[Client] ✓ Response verified successfully!\n");
}

}
