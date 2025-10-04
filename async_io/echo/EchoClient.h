#pragma once

#include "async/AsyncIoCoroutine.h"
#include <span>

class Reactor;

class EchoClient {
public:
    EchoClient(Reactor& reactor, int stdin_fd, int write_fd, int read_fd);
    
    AsyncIoCoroutine run();

private:
    Reactor& _reactor;
    int _stdin_fd;
    int _write_fd;
    int _read_fd;
    
    void log_input(std::span<const char> data);
    void log_write_result(size_t bytes_written);
    bool verify_and_log_echo(std::span<const char> sent, std::span<const char> received);
};