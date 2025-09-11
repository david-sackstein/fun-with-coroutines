#pragma once

// Coroutine-based calculator server.
// Reads arithmetic expressions from a pipe, evaluates each one, and writes
// the result back. The entire read/eval/write loop is a single coroutine
// that co_awaits async reads and writes, making the flow read like sequential code.
// Compare with the no-coroutines version which expresses the same loop as a chain of callbacks.

#include "common/reactor/Reactor.h"
#include "coroutines/return_types/FinalSuspendCoroutine.h"

namespace coroutines {

class CalcServer {
public:
    CalcServer(Reactor& reactor, int read_fd, int write_fd);

    // Start the server coroutine. It runs eagerly and suspends at each co_await.
    [[nodiscard]] FinalSuspendCoroutine run() const;

private:
    // Log the raw message bytes received from the client.
    static void log_received_message(const char *data, size_t size);

    // Throw if fewer bytes were written than expected.
    static void verify_write_complete(size_t expected, size_t actual);

    Reactor& _reactor;
    int _read_fd;   // pipe from client to server
    int _write_fd;  // pipe from server to client
};

}
