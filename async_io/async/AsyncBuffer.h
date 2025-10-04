#pragma once

#include "reactor/Reactor.h"
#include <coroutine>
#include <span>
#include <unistd.h>
#include <cerrno>
#include <type_traits>

// Default I/O functions (stateless, zero-size with [[no_unique_address]])
struct DefaultRead {
    ssize_t operator()(int fd, char* buf, size_t count) const {
        return ::read(fd, buf, count);
    }
};

struct DefaultWrite {
    ssize_t operator()(int fd, const char* buf, size_t count) const {
        return ::write(fd, buf, count);
    }
};

// Default stop conditions
struct SingleShot {
    bool operator()(std::span<const char>, size_t) const {
        return false;  // Always done after one I/O
    }
};

struct UntilFull {
    bool operator()(std::span<const char> buffer, size_t offset) const {
        return offset < buffer.size();  // Continue until buffer full
    }
};

template<char Delimiter>
struct UntilDelimiter {
    bool operator()(std::span<const char> buffer, size_t offset) const {
        if (offset == 0) return true;  // Need at least one byte
        return buffer[offset - 1] != Delimiter;  // Stop when delimiter found
    }
};

// Generic async I/O operation
// Mode: Reactor::FdMode::Read or Reactor::FdMode::Write
// StopCondition: Determines when to stop (returns true = need more data)
// IoFunc: I/O function (defaults to ::read or ::write based on Mode)
template<Reactor::FdMode Mode,
         typename StopCondition = SingleShot,
         typename IoFunc = std::conditional_t<Mode == Reactor::FdMode::Read, DefaultRead, DefaultWrite>>
struct AsyncBuffer {
    using CharType = std::conditional_t<Mode == Reactor::FdMode::Read, char, const char>;
    
    Reactor& reactor;
    int fd;
    std::span<CharType> buffer;
    [[no_unique_address]] StopCondition stop_condition;
    [[no_unique_address]] IoFunc io_func;
    
    // Constructor for default I/O and stop condition
    AsyncBuffer(Reactor& r, int f, std::span<CharType> b)
        requires (std::is_default_constructible_v<IoFunc> && 
                  std::is_default_constructible_v<StopCondition>)
        : reactor(r), fd(f), buffer(b), stop_condition{}, io_func{} {}
    
    // Constructor for custom stop condition, default I/O
    AsyncBuffer(Reactor& r, int f, std::span<CharType> b, StopCondition sc)
        requires std::is_default_constructible_v<IoFunc>
        : reactor(r), fd(f), buffer(b), stop_condition(sc), io_func{} {}
    
    // Constructor for custom I/O and stop condition
    AsyncBuffer(Reactor& r, int f, std::span<CharType> b, StopCondition sc, IoFunc func)
        : reactor(r), fd(f), buffer(b), stop_condition(sc), io_func(func) {}
    
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
        if (!needs_more_data()) {
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
        return io_func(fd, buffer.data() + offset, buffer.size() - offset);
    }
    
    bool should_retry(ssize_t n) const {
        return n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
    }
    
    bool should_stop(ssize_t n) const {
        return n <= 0;
    }
    
    bool needs_more_data() const {
        // Pass full buffer, not just filled portion
        return stop_condition(buffer, offset);
    }
    
    std::coroutine_handle<> handle;
    size_t offset = 0;
};

// Convenient aliases
template<typename IoFunc = DefaultRead>
using AsyncReadBuffer = AsyncBuffer<Reactor::FdMode::Read, SingleShot, IoFunc>;

template<typename IoFunc = DefaultWrite>
using AsyncWriteBuffer = AsyncBuffer<Reactor::FdMode::Write, SingleShot, IoFunc>;

template<typename IoFunc = DefaultRead>
using AsyncReadExact = AsyncBuffer<Reactor::FdMode::Read, UntilFull, IoFunc>;

template<typename IoFunc = DefaultWrite>
using AsyncWriteExact = AsyncBuffer<Reactor::FdMode::Write, UntilFull, IoFunc>;

template<char Delimiter, typename IoFunc = DefaultRead>
using AsyncReadUntil = AsyncBuffer<Reactor::FdMode::Read, UntilDelimiter<Delimiter>, IoFunc>;