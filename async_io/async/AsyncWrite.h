#pragma once

#include "reactor/Reactor.h"
#include <coroutine>

struct AsyncWrite {
    Reactor& reactor;
    int fd;

    bool await_ready() { return false; }
    
    void await_suspend(std::coroutine_handle<> h) {
        reactor.post(fd, Reactor::FdMode::Write, [=, this](int) {
            // Remove the handler (one-shot behavior)
            reactor.remove(fd, Reactor::FdMode::Write);
            // Resume the coroutine
            h.resume();
        });
    }
    
    void await_resume() {}
};

