#pragma once

#include "async/AsyncIoCoroutine.h"
#include <string>

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
};

