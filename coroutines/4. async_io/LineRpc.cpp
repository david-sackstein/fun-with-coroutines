#include "coroutines/4. async_io/LineRpc.h"

#include "common/io/print.h"
#include "coroutines/4. async_io/async/AsyncIo.h"

#include <format>
#include <stdexcept>

namespace coroutines {

AsyncIoCoroutine line_rpc(const async_io::LineTransport &transport, async_io::LineRpcCall &call) {
    const size_t written =
        co_await async_write_exact(transport.reactor, transport.write_fd, {call.request.data(), call.request.size()});

    if (written < call.request.size()) {
        throw std::runtime_error(std::format(
            "[Client] Failed to write all bytes to pipe_client_to_server! Expected {} bytes, wrote {} bytes",
            call.request.size(), written));
    }

    io::print("[Client] Wrote {} bytes to pipe_client_to_server\n", written);

    call.response_size =
        co_await async_read_until<'\n'>(transport.reactor, transport.read_fd, call.response);
}

}
