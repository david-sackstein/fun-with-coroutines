#pragma once

#include "reactor/Reactor.h"

#include <coroutine>
#include <span>
#include <unistd.h>
#include <cerrno>
#include <type_traits>

// ============================================================================
// Default I/O functions
// ============================================================================

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

// ============================================================================
// Stop Conditions
// ============================================================================

struct SingleShot {
    bool operator()(std::span<const char>, size_t) const {
        return false;
    }
};

struct UntilFull {
    bool operator()(std::span<const char> buffer, size_t offset) const {
        return offset < buffer.size();
    }
};

template<char Delimiter>
struct UntilDelimiter {
    bool operator()(std::span<const char> buffer, size_t offset) const {
        if (offset == 0) return true;
        return buffer[offset - 1] != Delimiter;
    }
};

// ============================================================================
// AsyncBuffer
// ============================================================================

template<Reactor::FdMode Mode,
    typename StopCondition = SingleShot,
    typename IoFunc = std::conditional_t<Mode == Reactor::FdMode::Read, DefaultRead, DefaultWrite>>
struct AsyncBuffer {
    using CharType = std::conditional_t<Mode == Reactor::FdMode::Read, char, const char>;

    Reactor& _reactor;
    int _fd;
    std::span<CharType> _buffer;
    std::coroutine_handle<> handle;
    size_t offset = 0;

    [[no_unique_address]] StopCondition stop_condition{};
    [[no_unique_address]] IoFunc io_func{};

    // Constructor using default stop_condition and io_func
    AsyncBuffer(Reactor& reactor, int fd, std::span<CharType> buffer)
        : _reactor(reactor), _fd(fd), _buffer(buffer) {}

    // Constructor for custom stop_condition
    AsyncBuffer(Reactor& reactor, int fd, std::span<CharType> buffer, StopCondition sc)
        : _reactor(reactor), _fd(fd), _buffer(buffer), stop_condition(sc) {}

    // Constructor for custom stop_condition and io_func
    AsyncBuffer(Reactor& reactor, int fd, std::span<CharType> buffer, StopCondition sc, IoFunc func)
        : _reactor(reactor), _fd(fd), _buffer(buffer), stop_condition(sc), io_func(func) {}

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
        _reactor.post(_fd, Mode, [this](int) {
            _reactor.remove(_fd, Mode);
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

        offset += n;

        if (needs_more_data()) {
            post_io();
        } else {
            handle.resume();
        }
    }

    ssize_t do_io() {
        return io_func(_fd, _buffer.data() + offset, _buffer.size() - offset);
    }

    bool should_retry(ssize_t n) const {
        return n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
    }

    bool should_stop(ssize_t n) const {
        return n <= 0;
    }

    bool needs_more_data() const {
        return stop_condition(_buffer, offset);
    }
};

// Convenient aliases

template<typename IoFunc = DefaultRead>
using AsyncReadBuffer = AsyncBuffer<Reactor::FdMode::Read, SingleShot, IoFunc>;

template<typename IoFunc = DefaultRead>
using AsyncReadExact = AsyncBuffer<Reactor::FdMode::Read, UntilFull, IoFunc>;

template<char Delimiter, typename IoFunc = DefaultRead>
using AsyncReadUntil = AsyncBuffer<Reactor::FdMode::Read, UntilDelimiter<Delimiter>, IoFunc>;

template<typename IoFunc = DefaultWrite>
using AsyncWriteBuffer = AsyncBuffer<Reactor::FdMode::Write, SingleShot, IoFunc>;

template<typename IoFunc = DefaultWrite>
using AsyncWriteExact = AsyncBuffer<Reactor::FdMode::Write, UntilFull, IoFunc>;
