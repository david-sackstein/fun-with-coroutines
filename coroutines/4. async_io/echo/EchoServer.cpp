#include "coroutines/4. async_io/async/AsyncIo.h"
#include "coroutines/4. async_io/echo/EchoServer.h"
#include "common/reactor/WorkGuard.h"

#include <print>
#include <stdexcept>

namespace coroutines {

EchoServer::EchoServer(Reactor &reactor, int read_fd, int write_fd)
    : _reactor(reactor), _read_fd(read_fd), _write_fd(write_fd) {}

AsyncIoCoroutine EchoServer::run() {
    WorkGuard guard(_reactor);

    std::print("[Server] Started\n");

    char buffer[256];

    while (true) {
        size_t total = co_await async_read_until<'\n'>(_reactor, _read_fd, buffer);

        if (total == 0) {
            std::print("[Server] EOF on pipe_client_to_server\n");
            break;
        }

        log_received_message(buffer, total);

        size_t written = co_await async_write_exact(_reactor, _write_fd, {buffer, total});
        verify_write_complete(total, written);
    }

    std::print("[Server] Finished\n");
}

void EchoServer::log_received_message(const char *data, size_t size) {
    std::print("[Server] Received: {}", std::string_view(data, size));
}

void EchoServer::verify_write_complete(size_t expected, size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format(
            "[Server] Failed to write all bytes to pipe_server_to_client! Expected {} bytes, wrote {} bytes", expected,
            actual));
    }
    std::print("[Server] Echoed {} bytes to pipe_server_to_client\n", actual);
}

}