#include "no-coroutines/4. async_io/LineRpc.h"

#include "common/io/print.h"
#include "no-coroutines/4. async_io/async/AsyncIoCallbacks.h"

#include <format>
#include <stdexcept>

namespace no_coroutines {

LineRpc::LineRpc(const async_io::LineTransport &transport) : _transport(transport) {}

void LineRpc::round_trip(async_io::LineRpcCall &call, std::function<void()> on_complete) {
    _call = &call;
    _on_complete = std::move(on_complete);
    async_write_request();
}

void LineRpc::async_write_request() const {
    post_write(_transport.reactor, _transport.write_fd, _call->request.data(), _call->request.size(),
               [this, size = _call->request.size()](const size_t actual) { on_write_complete(size, actual); });
}

void LineRpc::on_write_complete(const size_t expected, const size_t actual) const {
    verify_write_complete(expected, actual);
    async_read_response();
}

void LineRpc::async_read_response() const {
    post_read(_transport.reactor, _transport.read_fd, _call->response.data(), _call->response.size(),
              stop_at_newline_or_full(),
              [this](const size_t bytes_read) { on_read_complete(bytes_read); });
}

void LineRpc::on_read_complete(const size_t bytes_read) const {
    _call->response_size = bytes_read;
    _on_complete();
}

void LineRpc::verify_write_complete(const size_t expected, const size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format(
            "[Client] Failed to write all bytes to pipe_client_to_server! Expected {} bytes, wrote {} bytes", expected,
            actual));
    }
    io::print("[Client] Wrote {} bytes to pipe_client_to_server\n", actual);
}

}
