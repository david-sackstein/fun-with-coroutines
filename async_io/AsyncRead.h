#pragma once

#include "Selector.h"
#include <coroutine>

struct AsyncRead {
    Selector& selector;
    int fd;

    bool await_ready() { return false; }
    
    void await_suspend(std::coroutine_handle<> h) {
        selector.post(fd, Selector::FdMode::Read, [=, this](int) {
            // Remove the handler (one-shot behavior)
            selector.remove(fd, Selector::FdMode::Read);
            // Resume the coroutine
            h.resume();
        });
    }
    
    void await_resume() {}
};

struct AsyncWrite {
    Selector& selector;
    int fd;

    bool await_ready() { return false; }
    
    void await_suspend(std::coroutine_handle<> h) {
        selector.post(fd, Selector::FdMode::Write, [=, this](int) {
            // Remove the handler (one-shot behavior)
            selector.remove(fd, Selector::FdMode::Write);
            // Resume the coroutine
            h.resume();
        });
    }
    
    void await_resume() {}
};

