#pragma once

#include "common/reactor/Reactor.h"

namespace no_coroutines {

class EchoServer {
public:
    EchoServer(Reactor& reactor, int read_fd, int write_fd);

    void run();

private:
    Reactor& _reactor;
    int _read_fd;
    int _write_fd;
};

}
