#pragma once

#include "async_io/async/AsyncIoCoroutine.h"
#include "async_io/reactor/Reactor.h"

class EchoClient {
public:
    EchoClient(Reactor& reactor, int stdin_fd, int write_fd, int read_fd);
    
    AsyncIoCoroutine run();

private:
    Reactor& _reactor;
    int _stdin_fd;
    int _write_fd;
    int _read_fd;
    
    void log_input(const char *data, size_t size);
    void verify_write_complete(size_t expected, size_t actual);
    void verify_read_complete(size_t expected, size_t actual);
    void verify_and_log_echo(const char *sent, size_t sent_size, 
                             const char *received, size_t received_size);
};