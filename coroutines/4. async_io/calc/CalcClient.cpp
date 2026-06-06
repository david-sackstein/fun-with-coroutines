#include "common/async_io/CalcLine.h"
#include "common/io/print.h"
#include "common/reactor/WorkGuard.h"
#include "coroutines/4. async_io/async/AsyncIo.h"
#include "coroutines/4. async_io/calc/CalcClient.h"

#include <format>
#include <stdexcept>

namespace coroutines {

CalcClient::CalcClient(Reactor &reactor, const int stdin_fd, const int write_fd, const int read_fd)
    : _reactor(reactor), _stdin_fd(stdin_fd), _write_fd(write_fd), _read_fd(read_fd) {}

AsyncIoCoroutine CalcClient::run() const {
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

        const std::string expected = async_io::CalcLine::eval({write_buffer, total});
        const size_t received =
            co_await async_read_exact(_reactor, _read_fd, {read_buffer, expected.size()});

        verify_read_complete(expected.size(), received);
        verify_and_log_response({read_buffer, received}, expected);
    }

    io::print("[Client] Finished\n");
}

void CalcClient::log_input(const char *data, const size_t size) {
    io::print("[Client] Read from stdin: {}", std::string_view(data, size));
}

void CalcClient::verify_write_complete(const size_t expected, const size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format(
            "[Client] Failed to write all bytes to pipe_client_to_server! Expected {} bytes, wrote {} bytes", expected,
            actual));
    }
    io::print("[Client] Wrote {} bytes to pipe_client_to_server\n", actual);
}

void CalcClient::verify_read_complete(const size_t expected, const size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(
            std::format("[Client] Failed to read all bytes from pipe_server_to_client! Expected {} bytes, got {} bytes",
                        expected, actual));
    }
}

void CalcClient::verify_and_log_response(const std::string_view received, const std::string_view expected) {
    if (received != expected) {
        throw std::runtime_error("[Client] Response mismatch!");
    }
    io::print("[Client] Read from pipe_server_to_client: {}", received);
    io::print("[Client] ✓ Response verified successfully!\n");
}

}
