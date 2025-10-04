#pragma once

#include "reactor/Reactor.h"
#include <coroutine>
#include <span>
#include <unistd.h>

// Generic async read operation
// LoopUntilComplete: if false (default), reads once; if true, loops until all bytes read
template<bool LoopUntilComplete = false>
struct AsyncReadBuffer {
    Reactor& reactor;
    int fd;
    std::span<char> buffer;
    
    AsyncReadBuffer(Reactor& r, int f, std::span<char> b)
        : reactor(r), fd(f), buffer(b) {}
    
    bool await_ready() { return false; }
    
    void await_suspend(std::coroutine_handle<> h) {
        this->handle = h;
        offset = 0;
        read_next();
    }
    
    size_t await_resume() {
        return offset;  // Total bytes actually read
    }

private:
    void read_next() {
        if (offset >= buffer.size()) {
            // All bytes read, resume the coroutine
            handle.resume();
            return;
        }
        
        // Post read operation
        reactor.post(fd, Reactor::FdMode::Read, [this](int) {
            reactor.remove(fd, Reactor::FdMode::Read);
            
            ssize_t n = ::read(fd, buffer.data() + offset, buffer.size() - offset);
            
            if (n <= 0) {
                // EOF or error, stop reading
                handle.resume();
                return;
            }
            
            offset += n;
            
            // Continue reading if LoopUntilComplete is true
            if constexpr (LoopUntilComplete) {
                read_next();  // Loop: read more if needed
            } else {
                handle.resume();  // Single-shot: return immediately
            }
        });
    }
    
    std::coroutine_handle<> handle;
    size_t offset = 0;
};
