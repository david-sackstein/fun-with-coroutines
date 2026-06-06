#pragma once

#include "common/async_io/LineHandler.h"
#include "common/async_io/LineTransport.h"
#include "common/reactor/Reactor.h"
#include "common/reactor/WorkGuard.h"
#include "no-coroutines/4. async_io/LineRpc.h"

#include <memory>

namespace no_coroutines {

class EchoClient {
public:
    EchoClient(Reactor& reactor, int stdin_fd, int write_fd, int read_fd);

    void run();

private:
    Reactor& _reactor;
    int _stdin_fd;

    async_io::LineTransport _transport;
    LineRpc _line_rpc;

    std::unique_ptr<WorkGuard> _work_guard;

    char _write_buffer[256]{};
    char _read_buffer[256]{};
    async_io::LineRpcCall _rpc_call{{}, {}};

    void async_read_from_stdin();
    void on_read_complete(size_t bytes_read);

    void on_rpc_complete();

    static void log_input(const char *data, size_t size);
    static void verify_and_log_response(std::string_view request, std::string_view response,
                                        const async_io::LineHandler &handler);
};

}
