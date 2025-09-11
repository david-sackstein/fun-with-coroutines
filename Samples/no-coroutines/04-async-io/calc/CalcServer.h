#pragma once

// Callback-based calculator server.
// The read/eval/write loop is expressed as a chain of callbacks rather than
// sequential code. Compare with the coroutines version where the same logic
// is a single coroutine with co_await expressions.
//
// Callback chain:
//   run() -> async_read_message() -> on_read_complete()
//         -> async_write_response() -> on_write_complete()
//         -> async_read_message() (loop)

#include "common/reactor/Reactor.h"
#include "common/reactor/WorkGuard.h"

#include <memory>
#include <string>

namespace no_coroutines {

class CalcServer {
public:
    CalcServer(Reactor& reactor, int read_fd, int write_fd);

    // Create the WorkGuard and start the first async read.
    void run();

private:
    // Register an async read for the next message from the client.
    void async_read_message();
    // Called when a full newline-terminated message has been received.
    void on_read_complete(size_t bytes_read);

    // Log the raw message bytes received from the client.
    static void log_received_message(const char *data, size_t size);

    // Register an async write of the response back to the client.
    void async_write_response(size_t size);

    // Called when the write completes; verifies the count and loops back to read.
    void on_write_complete(size_t expected, size_t actual);

    // Throw if fewer bytes were written than expected.
    void verify_write_complete(size_t expected, size_t actual);

    Reactor& _reactor;
    int _read_fd;   // pipe from client to server
    int _write_fd;  // pipe from server to client

    // Held for the lifetime of the server to keep the Reactor running.
    std::unique_ptr<WorkGuard> _work_guard;

    char _buffer[256]{};   // reused for each incoming message
    std::string _response; // holds the evaluated result across the async write
};

}
