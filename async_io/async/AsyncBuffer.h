#pragma once

#include "reactor/Reactor.h"
#include <coroutine>
#include <span>
#include <unistd.h>
#include <cerrno>
#include <type_traits>

// Generic async I/O operation
// Mode: Reactor::FdMode::Read or Reactor::FdMode::Write
// LoopUntilComplete: if false (default), does one I/O; if true, loops until all bytes transferred
template<Reactor::FdMode Mode, bool LoopUntilComplete = false>
struct AsyncBuffer {
    using CharType = std::conditional_t<Mode == Reactor::FdMode::Read, char, const char>;
    
    Reactor& reactor;
    int fd;
    std::span<CharType> buffer;
    
    AsyncBuffer(Reactor& r, int f, std::span<CharType> b)
        : reactor(r), fd(f), buffer(b) {}
    
    bool await_ready() { return false; }
    
    void await_suspend(std::coroutine_handle<> h) {
        this->handle = h;
        offset = 0;
        wait_and_io();
    }
    
    size_t await_resume() {
        return offset;
    }

private:
    void wait_and_io() {
        if (offset >= buffer.size()) {
            handle.resume();
            return;
        }
        post_io();
    }
    
    void post_io() {
        reactor.post(fd, Mode, [this](int) {
            reactor.remove(fd, Mode);
            perform_io();
        });
    }
    
    void perform_io() {
        ssize_t n = do_io();
        
        if (should_retry(n)) {
            post_io();
            return;
        }
        
        if (should_stop(n)) {
            handle.resume();
            return;
        }
        
        // Success: n > 0
        offset += n;
        
        if (needs_more_data()) {
            post_io();
        } else {
            handle.resume();
        }
    }
    
    ssize_t do_io() {
        if constexpr (Mode == Reactor::FdMode::Read) {
            return ::read(fd, buffer.data() + offset, buffer.size() - offset);
        } else {
            return ::write(fd, buffer.data() + offset, buffer.size() - offset);
        }
    }
    
    bool should_retry(ssize_t n) const {
        return n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
    }
    
    bool should_stop(ssize_t n) const {
        return n <= 0;
    }
    
    bool needs_more_data() const {
        if constexpr (LoopUntilComplete) {
            return offset < buffer.size();
        } else {
            return false;
        }
    }
    
    std::coroutine_handle<> handle;
    size_t offset = 0;
};

// Convenient aliases
template<bool Loop = false>
using AsyncReadBuffer = AsyncBuffer<Reactor::FdMode::Read, Loop>;

template<bool Loop = false>
using AsyncWriteBuffer = AsyncBuffer<Reactor::FdMode::Write, Loop>;
