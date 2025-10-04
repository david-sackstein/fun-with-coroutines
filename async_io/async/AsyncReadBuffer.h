#pragma once

#include "reactor/Reactor.h"
#include <coroutine>
#include <span>
#include <unistd.h>
#include <cerrno>

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
        wait_and_read();  // Initial call with check
    }
    
    size_t await_resume() {
        return offset;
    }

private:
    // Called initially - checks if done before posting
    void wait_and_read() {
        if (offset >= buffer.size()) {
            handle.resume();
            return;
        }
        post_read();
    }
    
    // Just posts to reactor - no checks (caller already verified need)
    void post_read() {
        reactor.post(fd, Reactor::FdMode::Read, [this](int) {
            reactor.remove(fd, Reactor::FdMode::Read);
            perform_read();
        });
    }
    
    void perform_read() {
        ssize_t n = ::read(fd, buffer.data() + offset, buffer.size() - offset);
        
        if (should_retry(n)) {
            post_read();  // Retry: just re-post
            return;
        }
        
        if (should_stop(n)) {
            handle.resume();
            return;
        }
        
        // Success: n > 0
        offset += n;
        
        if (needs_more_data()) {
            post_read();  // Continue: just re-post
        } else {
            handle.resume();
        }
    }
    
    bool should_retry(ssize_t n) const {
        return n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
    }
    
    bool should_stop(ssize_t n) const {
        return n <= 0;  // EOF or error
    }
    
    bool needs_more_data() const {
        if constexpr (LoopUntilComplete) {
            return offset < buffer.size();
        } else {
            return false;  // Single-shot: always done after one read
        }
    }
    
    std::coroutine_handle<> handle;
    size_t offset = 0;
};