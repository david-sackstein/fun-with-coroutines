#pragma once

// Coroutine-based calculator client.
// Reads expressions from stdin, forwards them to the server, reads the response,
// and verifies it matches the locally evaluated result. Each step is a co_await,
// making the flow read like sequential blocking code.
// Compare with the no-coroutines version which expresses the same loop as a chain of callbacks.

#include "common/reactor/Reactor.h"
#include "coroutines/return_types/FinalSuspendCoroutine.h"

#include <string_view>

namespace coroutines {

class CalcClient {
public:
    CalcClient(Reactor& reactor, int stdin_fd, int write_fd, int read_fd);

    // Start the client coroutine. It runs eagerly and suspends at each co_await.
    [[nodiscard]] FinalSuspendCoroutine run() const;

private:
    // Log the raw expression read from stdin.
    static void log_input(const char *data, size_t size);

    // Throw if fewer bytes were written to the server pipe than expected.
    static void verify_write_complete(size_t expected, size_t actual);

    // Throw if fewer bytes were read from the server pipe than expected.
    static void verify_read_complete(size_t expected, size_t actual);

    // Throw if the response does not match the expected value; log it on success.
    static void verify_and_log_response(std::string_view received, std::string_view expected);

    Reactor& _reactor;
    int _stdin_fd;  // source of arithmetic expressions
    int _write_fd;  // pipe from client to server
    int _read_fd;   // pipe from server to client
};

}
