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
    ssize_t operator()(int fd, char *buf, size_t count) const {
        return ::read(fd, buf, count);
    }
};

struct DefaultWrite {
    ssize_t operator()(int fd, const char *buf, size_t count) const {
        return ::write(fd, buf, count);
    }
};

// ============================================================================
// Stop Conditions
// ============================================================================

struct SingleShot {
    bool operator()(std::span<const char>, size_t) const { return false; }
};

struct UntilFull {
    bool operator()(std::span<const char> buffer, size_t offset) const {
        return offset < buffer.size();
    }
};

template<char Delimiter>
struct UntilDelimiter {
    bool operator()(std::span<const char> buffer, size_t offset) const {
        return offset == 0 || buffer[offset - 1] != Delimiter;
    }
};

// ============================================================================
// AsyncBuffer
// ============================================================================

template<Reactor::FdMode Mode, typename StopCondition, typename IoFunc>
struct AsyncBuffer {
    using CharType = std::conditional_t<Mode == Reactor::FdMode::Read, char, const char>;

    Reactor &_reactor;
    int _fd;
    std::span<CharType> _buffer;
    std::coroutine_handle<> _handle;
    size_t _offset = 0;

    [[no_unique_address]] StopCondition stop_condition;
    [[no_unique_address]] IoFunc io_func;

    AsyncBuffer(Reactor &reactor, int fd, std::span<CharType> buffer, StopCondition sc, IoFunc func)
        : _reactor(reactor),
          _fd(fd),
          _buffer(buffer),
          stop_condition(sc),
          io_func(func) {}

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> handle) {
        _handle = handle;
        _offset = 0;
        wait_and_io();
    }

    size_t await_resume() {
        return _offset;
    }

private:
    void wait_and_io() {
        if (!needs_more_data()) {
            _handle.resume();
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
        ssize_t n = io_func(_fd, _buffer.data() + _offset, _buffer.size() - _offset);

        if (should_retry(n)) {
            post_io();
            return;
        }

        // Stop immediately on EOF or real error
        if (n <= 0) {
            _handle.resume();
            return;
        }

        // Success: n > 0
        _offset += n;

        if (needs_more_data()) {
            post_io();
        } else {
            _handle.resume();
        }
    }

    bool should_retry(ssize_t n) const {
        return n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
    }

    bool needs_more_data() const {
        return stop_condition(_buffer, _offset);
    }
};

// Read operations
inline auto async_read_buffer(Reactor &reactor, int fd, std::span<char> buffer) {
    return AsyncBuffer<Reactor::FdMode::Read, SingleShot, DefaultRead>{
        reactor, fd, buffer, SingleShot{}, DefaultRead{}
    };
}

inline auto async_read_exact(Reactor &reactor, int fd, std::span<char> buffer) {
    return AsyncBuffer<Reactor::FdMode::Read, UntilFull, DefaultRead>{
        reactor, fd, buffer, UntilFull{}, DefaultRead{}
    };
}

template<char Delimiter>
inline auto async_read_until(Reactor &reactor, int fd, std::span<char> buffer) {
    return AsyncBuffer<Reactor::FdMode::Read, UntilDelimiter<Delimiter>, DefaultRead>{
        reactor, fd, buffer, UntilDelimiter<Delimiter>{}, DefaultRead{}
    };
}

// Write operations
inline auto async_write_buffer(Reactor &reactor, int fd, std::span<const char> buffer) {
    return AsyncBuffer<Reactor::FdMode::Write, SingleShot, DefaultWrite>{
        reactor, fd, buffer, SingleShot{}, DefaultWrite{}
    };
}

inline auto async_write_exact(Reactor &reactor, int fd, std::span<const char> buffer) {
    return AsyncBuffer<Reactor::FdMode::Write, UntilFull, DefaultWrite>{
        reactor, fd, buffer, UntilFull{}, DefaultWrite{}
    };
}