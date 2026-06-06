#include "EchoClient.h"
#include "common/io/print.h"
#include "no-coroutines/4. async_io/ReactorIo.h"

#include <cstring>
#include <format>
#include <stdexcept>

#include <unistd.h>

namespace no_coroutines {

EchoClient::EchoClient(Reactor &reactor, const int stdin_fd, const int write_fd, const int read_fd)
    : _reactor(reactor), _stdin_fd(stdin_fd), _write_fd(write_fd), _read_fd(read_fd) {}

void EchoClient::run() {
    _work_guard = std::make_unique<WorkGuard>(_reactor);
    io::print("[Client] Started\n");
    async_read_from_stdin();
}

// ============================================================================
// Async read from stdin until newline
// ============================================================================

void EchoClient::async_read_from_stdin() {
    io::print("[Client] Waiting for input...\n");

    post_read(_reactor, _stdin_fd, _write_buffer, sizeof(_write_buffer),
              stop_at_newline_or_full(),
              [this](const size_t bytes_read) { on_read_complete(bytes_read); });
}

void EchoClient::on_read_complete(const size_t bytes_read) {
    if (bytes_read == 0) {
        io::print("[Client] EOF on stdin\n");
        close(_write_fd);
        io::print("[Client] Finished\n");
        _work_guard.reset();  // Release work guard
        return;
    }

    log_input(_write_buffer, bytes_read);
    async_write_to_server(bytes_read);
}

// ============================================================================
// Async write exact amount to server
// ============================================================================

void EchoClient::async_write_to_server(const size_t size) {
    post_write(_reactor, _write_fd, _write_buffer, size,
               [this, size](const size_t actual) { on_write_complete(size, actual); });
}

void EchoClient::on_write_complete(const size_t expected, const size_t actual) {
    verify_write_complete(expected, actual);
    async_read_echo(expected);
}

// ============================================================================
// Async read exact amount from server
// ============================================================================

void EchoClient::async_read_echo(const size_t size) {
    post_read(_reactor, _read_fd, _read_buffer, size,
              stop_at_exact_bytes(),
              [this, size](const size_t actual) { on_read_echo_complete(size, actual); });
}

void EchoClient::on_read_echo_complete(const size_t expected, const size_t actual) {
    verify_read_complete(expected, actual);
    verify_and_log_echo(_write_buffer, expected, _read_buffer, actual);

    // Continue the loop - read next input
    async_read_from_stdin();
}

// ============================================================================
// Utility methods
// ============================================================================

void EchoClient::log_input(const char *data, const size_t size) {
    io::print("[Client] Read from stdin: {}", std::string_view(data, size));
}

void EchoClient::verify_write_complete(const size_t expected, const size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format(
            "[Client] Failed to write all bytes to pipe_client_to_server! Expected {} bytes, wrote {} bytes", expected,
            actual));
    }
    io::print("[Client] Wrote {} bytes to pipe_client_to_server\n", actual);
}

void EchoClient::verify_read_complete(const size_t expected, const size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(
            std::format("[Client] Failed to read all bytes from pipe_server_to_client! Expected {} bytes, got {} bytes",
                        expected, actual));
    }
}

void EchoClient::verify_and_log_echo(const char *sent, const size_t sent_size,
                                     const char *received, const size_t received_size) {
    if (std::memcmp(sent, received, sent_size) != 0) {
        throw std::runtime_error("[Client] Echo mismatch!");
    }
    io::print("[Client] Read from pipe_server_to_client: {}", std::string_view(received, received_size));
    io::print("[Client] ✓ Echo verified successfully!\n");
}

}
