#pragma once

#include "coroutines/4. async_io/async/AsyncIoCoroutine.h"
#include "common/async_io/LineHandler.h"
#include "common/reactor/Reactor.h"

namespace coroutines {

class EchoClient {
public:
    EchoClient(Reactor& reactor, int stdin_fd, int write_fd, int read_fd);

    [[nodiscard]] AsyncIoCoroutine run() const;

private:
    Reactor& _reactor;
    int _stdin_fd;
    int _write_fd;
    int _read_fd;

    static void log_input(const char *data, size_t size);
    static void verify_and_log_response(std::string_view request, std::string_view response,
                                        const async_io::LineHandler &handler);
};

}
