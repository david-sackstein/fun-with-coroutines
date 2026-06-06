#include "EchoClient.h"
#include "common/async_io/LineHandler.h"
#include "common/async_io/LineTransport.h"
#include "common/io/print.h"
#include "no-coroutines/4. async_io/async/AsyncIoCallbacks.h"

#include <stdexcept>

#include <unistd.h>

namespace no_coroutines {

EchoClient::EchoClient(Reactor &reactor, const int stdin_fd, const int write_fd, const int read_fd)
    : _reactor(reactor),
      _stdin_fd(stdin_fd),
      _transport{reactor, read_fd, write_fd},
      _line_rpc(_transport) {}

void EchoClient::run() {
    _work_guard = std::make_unique<WorkGuard>(_reactor);
    io::print("[Client] Started\n");
    async_read_from_stdin();
}

void EchoClient::async_read_from_stdin() {
    io::print("[Client] Waiting for input...\n");

    post_read(_reactor, _stdin_fd, _write_buffer, sizeof(_write_buffer),
              stop_at_newline_or_full(),
              [this](const size_t bytes_read) { on_read_complete(bytes_read); });
}

void EchoClient::on_read_complete(const size_t bytes_read) {
    if (bytes_read == 0) {
        io::print("[Client] EOF on stdin\n");
        close(_transport.write_fd);
        io::print("[Client] Finished\n");
        _work_guard.reset();
        return;
    }

    log_input(_write_buffer, bytes_read);

    _rpc_call = async_io::LineRpcCall{{_write_buffer, bytes_read}, _read_buffer};
    _line_rpc.round_trip(_rpc_call, [this] { on_rpc_complete(); });
}

void EchoClient::on_rpc_complete() {
    verify_and_log_response(_rpc_call.request, {_rpc_call.response.data(), _rpc_call.response_size},
                            async_io::echo_line_handler());

    async_read_from_stdin();
}

void EchoClient::log_input(const char *data, const size_t size) {
    io::print("[Client] Read from stdin: {}", std::string_view(data, size));
}

void EchoClient::verify_and_log_response(const std::string_view request, const std::string_view response,
                                         const async_io::LineHandler &handler) {
    if (!async_io::response_matches(request, response, handler)) {
        throw std::runtime_error("[Client] Echo mismatch!");
    }
    io::print("[Client] Read from pipe_server_to_client: {}", response);
    io::print("[Client] ✓ Echo verified successfully!\n");
}

}
