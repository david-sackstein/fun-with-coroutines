#include "CalcServer.h"
#include "common/async_io/CalcLine.h"
#include "common/io/print.h"
#include "no-coroutines/04-async-io/async/AsyncIo.h"

#include <format>
#include <stdexcept>

namespace no_coroutines {

CalcServer::CalcServer(Reactor &reactor, const int read_fd, const int write_fd)
    : _reactor(reactor), _read_fd(read_fd), _write_fd(write_fd) {}

void CalcServer::run() {
    _work_guard = std::make_unique<WorkGuard>(_reactor);
    io::print("[Server] Started\n");
    async_read_message();
}

// ============================================================================
// Async read from client until newline
// ============================================================================

void CalcServer::async_read_message() {
    post_read(_reactor, _read_fd, _buffer, sizeof(_buffer),
              stop_at_newline_or_full(),
              [this](const size_t bytes_read) { on_read_complete(bytes_read); });
}

void CalcServer::on_read_complete(const size_t bytes_read) {
    if (bytes_read == 0) {
        io::print("[Server] EOF on pipe_client_to_server\n");
        io::print("[Server] Finished\n");
        _work_guard.reset();  // Release work guard
        return;
    }

    log_received_message(_buffer, bytes_read);

    _response = async_io::CalcLine::eval({_buffer, bytes_read});
    async_write_response(_response.size());
}

// ============================================================================
// Async write exact amount back to client
// ============================================================================

void CalcServer::async_write_response(const size_t size) {
    post_write(_reactor, _write_fd, _response.data(), size,
               [this, size](const size_t actual) { on_write_complete(size, actual); });
}

void CalcServer::on_write_complete(const size_t expected, const size_t actual) {
    verify_write_complete(expected, actual);

    // Continue the loop - read next message
    async_read_message();
}

// ============================================================================
// Utility methods
// ============================================================================

void CalcServer::log_received_message(const char *data, const size_t size) {
    io::print("[Server] Received: {}", std::string_view(data, size));
}

void CalcServer::verify_write_complete(const size_t expected, const size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format(
            "[Server] Failed to write all bytes to pipe_server_to_client! Expected {} bytes, wrote {} bytes", expected,
            actual));
    }
    io::print("[Server] Responded {} bytes to pipe_server_to_client\n", actual);
}

}
