#pragma once

#include "coroutines/4. async_io/async/AsyncIoCoroutine.h"
#include "common/reactor/Reactor.h"

namespace coroutines {

class EchoServer {
public:
    EchoServer(Reactor& reactor, int read_fd, int write_fd);
    
    AsyncIoCoroutine run();

private:
    Reactor& _reactor;
    int _read_fd;
    int _write_fd;
    
    static void log_received_message(const char *data, size_t size);
    static void verify_write_complete(size_t expected, size_t actual);
};

}