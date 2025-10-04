#pragma once

#include "reactor/Reactor.h"
#include <coroutine>
#include <span>
#include <unistd.h>
#include <cerrno>

// Generic async write operation
// LoopUntilComplete: if false (default), writes once; if true, loops until all bytes written
template<bool LoopUntilComplete = false>
struct AsyncWriteBuffer {
    Reactor& reactor;
    int fd;
    std::span<const char> buffer;
    
    AsyncWriteBuffer(Reactor& r, int f, std::span<const char> b)
        : reactor(r), fd(f), buffer(b) {}
    
    bool await_ready() { return false; }
    
    void await_suspend(std::coroutine_handle<> h) {
        this->handle = h;
        offset = 0;
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
        
        // Post write operation
        reactor.post(fd, Reactor::FdMode::Write, [this](int) {
            reactor.remove(fd, Reactor::FdMode::Write);
            
            ssize_t n = ::write(fd, buffer.data() + offset, buffer.size() - offset);
            
            if (n < 0) {
                // Check for expected non-blocking errors
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                    // FD not ready (EAGAIN/EWOULDBLOCK) or interrupted (EINTR)
                    // In both cases: re-register with reactor and retry
                    write_next();
                    return;
                }
                
                // Real error - stop and resume with current offset
                handle.resume();
                return;
            }
            
            if (n == 0) {
                // Unusual for write, but handle it - stop and resume
                handle.resume();
                return;
            }
            
            offset += n;
            
            // Continue writing if LoopUntilComplete is true
            if constexpr (LoopUntilComplete) {
                write_next();  // Loop: write more if needed
            } else {
                handle.resume();  // Single-shot: return immediately
            }
        });
    }
    
    std::coroutine_handle<> handle;
    size_t offset = 0;
};
