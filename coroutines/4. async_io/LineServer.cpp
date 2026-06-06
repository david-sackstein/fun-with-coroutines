#include "coroutines/4. async_io/LineServer.h"

#include "common/io/print.h"
#include "common/reactor/WorkGuard.h"
#include "coroutines/4. async_io/async/AsyncIo.h"

#include <format>
#include <stdexcept>

namespace coroutines {

AsyncIoCoroutine run_line_server(const async_io::LineTransport &transport, async_io::LineHandler handler) {
    const WorkGuard guard(transport.reactor);

    io::print("[Server] Started\n");

    char request[256];

    while (true) {
        const size_t request_size =
            co_await async_read_until<'\n'>(transport.reactor, transport.read_fd, request);

        if (request_size == 0) {
            io::print("[Server] EOF on pipe_client_to_server\n");
            break;
        }

        io::print("[Server] Received: {}", std::string_view(request, request_size));

        const std::string response = handler({request, request_size});
        const size_t written =
            co_await async_write_exact(transport.reactor, transport.write_fd, {response.data(), response.size()});

        if (written < response.size()) {
            throw std::runtime_error(std::format(
                "[Server] Failed to write all bytes to pipe_server_to_client! Expected {} bytes, wrote {} bytes",
                response.size(), written));
        }

        io::print("[Server] Responded {} bytes to pipe_server_to_client\n", written);
    }

    io::print("[Server] Finished\n");
}

}
