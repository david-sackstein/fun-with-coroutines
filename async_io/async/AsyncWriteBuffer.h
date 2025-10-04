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
        wait_and_write();  // Initial call with check
    }
    
    size_t await_resume() {
        return offset;
    }

private:
    // Called initially - checks if done before posting
    void wait_and_write() {
        if (offset >= buffer.size()) {
            handle.resume();
            return;
        }
        post_write();
    }
    
    // Just posts to reactor - no checks (caller already verified need)
    void post_write() {
        reactor.post(fd, Reactor::FdMode::Write, [this](int) {
            reactor.remove(fd, Reactor::FdMode::Write);
            perform_write();
        });
    }
    
    void perform_write() {
        ssize_t n = ::write(fd, buffer.data() + offset, buffer.size() - offset);
        
        if (should_retry(n)) {
            post_write();  // Retry: just re-post
            return;
        }
        
        if (should_stop(n)) {
            handle.resume();
            return;
        }
        
        // Success: n > 0
        offset += n;
        
        if (needs_more_data()) {
            post_write();  // Continue: just re-post
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
            return false;  // Single-shot: always done after one write
        }
    }
    
    std::coroutine_handle<> handle;
    size_t offset = 0;
};