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
    bool check_write_complete(size_t expected, size_t actual);
    bool check_read_complete(size_t expected, size_t actual);
    bool verify_and_log_echo(std::span<const char> sent, std::span<const char> received);
};