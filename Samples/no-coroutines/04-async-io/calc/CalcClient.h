#pragma once

// Callback-based calculator client.
// The stdin-forward-verify loop is expressed as a chain of callbacks rather than
// sequential code. Compare with the coroutines version where the same logic
// is a single coroutine with co_await expressions.
//
// Callback chain:
//   run() -> async_read_from_stdin() -> on_read_complete()
//         -> async_write_to_server() -> on_write_complete()
//         -> async_read_response()   -> on_read_response_complete()
//         -> async_read_from_stdin() (loop)

#include "common/reactor/Reactor.h"
#include "common/reactor/WorkGuard.h"

#include <memory>
#include <string>
#include <string_view>

namespace no_coroutines {

class CalcClient {
public:
    CalcClient(Reactor& reactor, int stdin_fd, int write_fd, int read_fd);

    // Create the WorkGuard and start the first async read from stdin.
    void run();

private:
    // Register an async read for the next line from stdin.
    void async_read_from_stdin();

    // Called when a full newline-terminated line has been read from stdin.
    void on_read_complete(size_t bytes_read);

    // Log the raw expression read from stdin.
    static void log_input(const char *data, size_t size);

    // Register an async write of the expression to the server pipe.
    void async_write_to_server(size_t size);

    // Called when the write completes; kick off the async response read.
    void on_write_complete(size_t expected, size_t actual);

    // Throw if fewer bytes were written to the server pipe than expected.
    void verify_write_complete(size_t expected, size_t actual);

    // Register an async read of exactly size bytes from the server pipe.
    void async_read_response(size_t size);

    // Called when the response has been fully received; verify and loop back.
    void on_read_response_complete(size_t expected, size_t actual);

    // Throw if fewer bytes were received than expected.
    void verify_read_complete(size_t expected, size_t actual);

    // Throw if the response does not match the expected value; log success otherwise.
    void verify_and_log_response(std::string_view received, std::string_view expected);

    Reactor& _reactor;
    int _stdin_fd;  // source of arithmetic expressions
    int _write_fd;  // pipe from client to server
    int _read_fd;   // pipe from server to client

    // Held for the lifetime of the client to keep the Reactor running.
    std::unique_ptr<WorkGuard> _work_guard;

    char _write_buffer[256]{};  // reused for each stdin line and server write
    char _read_buffer[256]{};   // reused for each server response
    // The locally evaluated result, held across the async read so on_read_response_complete can compare.
    std::string _expected_response;
};

}
