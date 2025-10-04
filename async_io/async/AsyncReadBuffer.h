#pragma once

#include "reactor/Reactor.h"
#include <coroutine>
#include <span>
#include <unistd.h>

struct AsyncReadBuffer {
    Reactor& reactor;
    int fd;
    std::span<char> buffer;
    
    AsyncReadBuffer(Reactor& s, int f, std::span<char> b)
        : reactor(s), fd(f), buffer(b) {}
    
    bool await_ready() { return false; }
    
    void await_suspend(std::coroutine_handle<> h) {
        reactor.post(fd, Reactor::FdMode::Read, [=, this](int) {
            // Remove the handler (one-shot behavior)
            reactor.remove(fd, Reactor::FdMode::Read);
            
            // Perform the read into caller-provided buffer
            bytes_read = ::read(fd, buffer.data(), buffer.size());
            
            // Resume the coroutine
            h.resume();
        });
    }
    
    ssize_t await_resume() {
        return bytes_read;
    }

private:
    ssize_t bytes_read = 0;
};

