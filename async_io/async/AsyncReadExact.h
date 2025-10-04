#pragma once

#include "reactor/Reactor.h"
#include "AsyncReadBuffer.h"
#include <coroutine>
#include <span>

// Reads exactly the specified number of bytes (or until EOF/error)
// Loops internally until all bytes are read
struct AsyncReadExact {
    Reactor& reactor;
    int fd;
    std::span<char> buffer;
    
    AsyncReadExact(Reactor& r, int f, std::span<char> b)
        : reactor(r), fd(f), buffer(b) {}
    
    bool await_ready() { return false; }
    
    void await_suspend(std::coroutine_handle<> h) {
        // Store the coroutine handle for later resumption
        this->handle = h;
        offset = 0;
        // Start the first read
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
        
        // Read more bytes
        reactor.post(fd, Reactor::FdMode::Read, [this](int) {
            reactor.remove(fd, Reactor::FdMode::Read);
            
            ssize_t n = ::read(fd, buffer.data() + offset, buffer.size() - offset);
            
            if (n <= 0) {
                // EOF or error, stop reading
                handle.resume();
                return;
            }
            
            offset += n;
            
            // Continue reading if needed
            read_next();
        });
    }
    
    std::coroutine_handle<> handle;
    size_t offset = 0;
};

