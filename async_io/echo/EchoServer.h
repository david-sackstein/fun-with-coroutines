#pragma once

#include "async/AsyncIoCoroutine.h"
#include <span>

class Reactor;

class EchoServer {
public:
    EchoServer(Reactor& reactor, int read_fd, int write_fd);
    
    AsyncIoCoroutine run();

private:
    Reactor& _reactor;
    int _read_fd;
    int _write_fd;
    
    void log_received_message(std::span<const char> data);
    void log_echo_result(size_t bytes_written);
};