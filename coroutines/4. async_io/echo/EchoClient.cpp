#include "common/async_io/LineHandler.h"
#include "common/async_io/LineTransport.h"
#include "common/io/print.h"
#include "common/reactor/WorkGuard.h"
#include "coroutines/4. async_io/async/AsyncIo.h"
#include "coroutines/4. async_io/LineRpc.h"
#include "coroutines/4. async_io/echo/EchoClient.h"

#include <stdexcept>

#include <unistd.h>

namespace coroutines {

EchoClient::EchoClient(Reactor &reactor, const int stdin_fd, const int write_fd, const int read_fd)
    : _reactor(reactor), _stdin_fd(stdin_fd), _write_fd(write_fd), _read_fd(read_fd) {}

AsyncIoCoroutine EchoClient::run() const {
    const WorkGuard guard(_reactor);
    const async_io::LineHandler handler = async_io::echo_line_handler();
    const async_io::LineTransport transport{_reactor, _read_fd, _write_fd};

    io::print("[Client] Started\n");

    char write_buffer[256];
    char read_buffer[256];

    while (true) {
        io::print("[Client] Waiting for input...\n");
        const size_t request_size = co_await async_read_until<'\n'>(_reactor, _stdin_fd, write_buffer);

        if (request_size == 0) {
            io::print("[Client] EOF on stdin\n");
            close(_write_fd);
            break;
        }

        log_input(write_buffer, request_size);

        async_io::LineRpcCall call{{write_buffer, request_size}, read_buffer};
        co_await line_rpc(transport, call);

        verify_and_log_response(call.request, {call.response.data(), call.response_size}, handler);
    }

    io::print("[Client] Finished\n");
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
