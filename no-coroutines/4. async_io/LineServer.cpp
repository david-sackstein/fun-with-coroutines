#include "no-coroutines/4. async_io/LineServer.h"

#include "common/io/print.h"
#include "no-coroutines/4. async_io/async/AsyncIoCallbacks.h"

#include <format>
#include <stdexcept>

namespace no_coroutines {

LineServer::LineServer(const async_io::LineTransport &transport) : _transport(transport) {}

void LineServer::run(async_io::LineHandler handler) {
    _handler = std::move(handler);
    _work_guard = std::make_unique<WorkGuard>(_transport.reactor);
    io::print("[Server] Started\n");
    async_read_message();
}

void LineServer::async_read_message() {
    post_read(_transport.reactor, _transport.read_fd, _request_buffer, sizeof(_request_buffer),
              stop_at_newline_or_full(),
              [this](const size_t bytes_read) { on_read_complete(bytes_read); });
}

void LineServer::on_read_complete(const size_t bytes_read) {
    if (bytes_read == 0) {
        io::print("[Server] EOF on pipe_client_to_server\n");
        io::print("[Server] Finished\n");
        _work_guard.reset();
        return;
    }

    io::print("[Server] Received: {}", std::string_view(_request_buffer, bytes_read));

    _response = _handler({_request_buffer, bytes_read});
    async_write_response(_response.size());
}

void LineServer::async_write_response(const size_t size) {
    post_write(_transport.reactor, _transport.write_fd, _response.data(), size,
               [this, size](const size_t actual) { on_write_complete(size, actual); });
}

void LineServer::on_write_complete(const size_t expected, const size_t actual) {
    verify_write_complete(expected, actual);
    async_read_message();
}

void LineServer::verify_write_complete(const size_t expected, const size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format(
            "[Server] Failed to write all bytes to pipe_server_to_client! Expected {} bytes, wrote {} bytes", expected,
            actual));
    }
    io::print("[Server] Responded {} bytes to pipe_server_to_client\n", actual);
}

}
