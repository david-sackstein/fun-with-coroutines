#include "common/async_io/CalcLine.h"
#include "common/io/print.h"
#include "common/reactor/WorkGuard.h"
#include "coroutines/04-async-io/async/AsyncIo.h"
#include "coroutines/04-async-io/calc/CalcServer.h"

#include <format>
#include <stdexcept>

namespace coroutines {

// Store the reactor and pipe file descriptors.
CalcServer::CalcServer(Reactor &reactor, const int read_fd, const int write_fd)
    : _reactor(reactor), _read_fd(read_fd), _write_fd(write_fd) {}

// Read-eval-write loop: each iteration reads one expression, evaluates it, and sends the result.
FinalSuspendCoroutine CalcServer::run() const {
    // Keep the reactor running for the lifetime of this coroutine.
    const WorkGuard guard(_reactor);

    io::print("[Server] Started\n");

    char buffer[256];

    while (true) {
        // Suspend until a complete newline-terminated expression arrives.
        const size_t total = co_await async_read_until<'\n'>(_reactor, _read_fd, buffer);

        if (total == 0) {
            // The client closed the write end of the pipe. Stop the server.
            io::print("[Server] EOF on pipe_client_to_server\n");
            break;
        }

        log_received_message(buffer, total);

        const std::string response = async_io::CalcLine::eval({buffer, total});

        // Suspend until all response bytes have been written.
        const size_t written =
            co_await async_write_exact(_reactor, _write_fd, {response.data(), response.size()});
        verify_write_complete(response.size(), written);
    }

    io::print("[Server] Finished\n");
}

// Print the raw message received from the client pipe.
void CalcServer::log_received_message(const char *data, const size_t size) {
    io::print("[Server] Received: {}", std::string_view(data, size));
}

// Verify all bytes were written; throw if the write was short.
void CalcServer::verify_write_complete(const size_t expected, const size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format(
            "[Server] Failed to write all bytes to pipe_server_to_client! Expected {} bytes, wrote {} bytes",
            expected, actual));
    }
    io::print("[Server] Responded {} bytes to pipe_server_to_client\n", actual);
}

}
