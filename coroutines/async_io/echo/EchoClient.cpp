#include "coroutines/async_io/echo/EchoClient.h"
#include "common/reactor/WorkGuard.h"
#include "coroutines/async_io/async/AsyncIo.h"

#include <print>
#include <stdexcept>

namespace coroutines {

EchoClient::EchoClient(Reactor &reactor, int stdin_fd, int write_fd, int read_fd)
    : _reactor(reactor), _stdin_fd(stdin_fd), _write_fd(write_fd), _read_fd(read_fd) {}

AsyncIoCoroutine EchoClient::run() {
    WorkGuard guard(_reactor);

    std::print("[Client] Started\n");

    char write_buffer[256];
    char read_buffer[256];

    while (true) {
        std::print("[Client] Waiting for input...\n");
        size_t total = co_await async_read_until<'\n'>(_reactor, _stdin_fd, write_buffer);

        if (total == 0) {
            std::print("[Client] EOF on stdin\n");
            close(_write_fd);
            break;
        }

        log_input(write_buffer, total);

        size_t written = co_await async_write_exact(_reactor, _write_fd, {write_buffer, total});

        verify_write_complete(total, written);

        size_t echoed = co_await async_read_exact(_reactor, _read_fd, {read_buffer, total});

        verify_read_complete(total, echoed);

        verify_and_log_echo(write_buffer, total, read_buffer, echoed);
    }

    std::print("[Client] Finished\n");
}

void EchoClient::log_input(const char *data, size_t size) {
    std::print("[Client] Read from stdin: {}", std::string_view(data, size));
}

void EchoClient::verify_write_complete(size_t expected, size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format(
            "[Client] Failed to write all bytes to pipe_client_to_server! Expected {} bytes, wrote {} bytes", expected,
            actual));
    }
    std::print("[Client] Wrote {} bytes to pipe_client_to_server\n", actual);
}

void EchoClient::verify_read_complete(size_t expected, size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(
            std::format("[Client] Failed to read all bytes from pipe_server_to_client! Expected {} bytes, got {} bytes",
                        expected, actual));
    }
}

void EchoClient::verify_and_log_echo(const char *sent, size_t sent_size,
                                     const char *received, size_t received_size) {
    if (std::memcmp(sent, received, sent_size) != 0) {
        throw std::runtime_error("[Client] Echo mismatch!");
    }
    std::print("[Client] Read from pipe_server_to_client: {}", std::string_view(received, received_size));
    std::print("[Client] ✓ Echo verified successfully!\n");
}

}