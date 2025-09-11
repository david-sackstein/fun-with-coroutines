#pragma once

#include "async_io/async/AsyncIoCoroutine.h"
#include "async_io/reactor/Reactor.h"

class EchoServer {
public:
    EchoServer(Reactor& reactor, int read_fd, int write_fd);
    
    AsyncIoCoroutine run();

private:
    Reactor& _reactor;
    int _read_fd;
    int _write_fd;
    
    void log_received_message(const char *data, size_t size);
    void verify_write_complete(size_t expected, size_t actual);
};