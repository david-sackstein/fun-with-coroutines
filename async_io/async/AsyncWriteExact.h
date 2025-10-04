#pragma once

#include "reactor/Reactor.h"
#include "AsyncWriteBuffer.h"
#include <coroutine>
#include <span>

// Writes exactly the specified number of bytes (or until error)
// Loops internally until all bytes are written
struct AsyncWriteExact {
    Reactor& reactor;
    int fd;
    std::span<const char> buffer;
    
    AsyncWriteExact(Reactor& r, int f, std::span<const char> b)
        : reactor(r), fd(f), buffer(b) {}
    
    bool await_ready() { return false; }
    
    void await_suspend(std::coroutine_handle<> h) {
        // Store the coroutine handle for later resumption
        this->handle = h;
        offset = 0;
        // Start the first write
        write_next();
    }
    
    size_t await_resume() {
        return offset;  // Total bytes actually written
    }

private:
    void write_next() {
        if (offset >= buffer.size()) {
            // All bytes written, resume the coroutine
            handle.resume();
            return;
        }
        
        // Write more bytes
        reactor.post(fd, Reactor::FdMode::Write, [this](int) {
            reactor.remove(fd, Reactor::FdMode::Write);
            
            ssize_t n = ::write(fd, buffer.data() + offset, buffer.size() - offset);
            
            if (n <= 0) {
                // Error, stop writing
                handle.resume();
                return;
            }
            
            offset += n;
            
            // Continue writing if needed
            write_next();
        });
    }
    
    std::coroutine_handle<> handle;
    size_t offset = 0;
};

