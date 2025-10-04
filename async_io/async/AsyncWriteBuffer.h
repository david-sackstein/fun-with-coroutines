#pragma once

#include "reactor/Reactor.h"
#include <coroutine>
#include <span>
#include <unistd.h>

struct AsyncWriteBuffer {
    Reactor& reactor;
    int fd;
    std::span<const char> buffer;
    
    AsyncWriteBuffer(Reactor& s, int f, std::span<const char> b)
        : reactor(s), fd(f), buffer(b) {}
    
    bool await_ready() { return false; }
    
    void await_suspend(std::coroutine_handle<> h) {
        reactor.post(fd, Reactor::FdMode::Write, [=, this](int) {
            // Remove the handler (one-shot behavior)
            reactor.remove(fd, Reactor::FdMode::Write);
            
            // Perform the write from caller-provided buffer
            bytes_written = ::write(fd, buffer.data(), buffer.size());
            
            // Resume the coroutine
            h.resume();
        });
    }
    
    ssize_t await_resume() {
        return bytes_written;
    }

private:
    ssize_t bytes_written = 0;
};

