#include "common/io/print.h"
#include "common/reactor/WorkGuard.h"
#include "coroutines/5. async_io/async/AsyncIo.h"
#include "coroutines/5. async_io/echo/EchoClient.h"

#include <cstring>
#include <format>
#include <stdexcept>

namespace coroutines {

EchoClient::EchoClient(Reactor &reactor, const int stdin_fd, const int write_fd, const int read_fd)
    : _reactor(reactor), _stdin_fd(stdin_fd), _write_fd(write_fd), _read_fd(read_fd) {}

AsyncIoCoroutine EchoClient::run() const {
    const WorkGuard guard(_reactor);

    io::print("[Client] Started\n");

    char write_buffer[256];
    char read_buffer[256];

    while (true) {
        io::print("[Client] Waiting for input...\n");
        const size_t total = co_await async_read_until<'\n'>(_reactor, _stdin_fd, write_buffer);

        if (total == 0) {
            io::print("[Client] EOF on stdin\n");
            close(_write_fd);
            break;
        }

        log_input(write_buffer, total);

        const size_t written = co_await async_write_exact(_reactor, _write_fd, {write_buffer, total});

        verify_write_complete(total, written);

        const size_t echoed = co_await async_read_exact(_reactor, _read_fd, {read_buffer, total});

        verify_read_complete(total, echoed);

        verify_and_log_echo(write_buffer, total, read_buffer, echoed);
    }

    io::print("[Client] Finished\n");
}

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
