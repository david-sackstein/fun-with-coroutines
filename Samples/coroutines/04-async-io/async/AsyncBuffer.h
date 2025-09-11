#pragma once

#include "common/reactor/Reactor.h"

#include <coroutine>
#include <cerrno>
#include <span>

namespace coroutines {

// Awaitable that performs non-blocking I/O through the Reactor until a stop condition is met.
//
// co_await on an AsyncBuffer suspends the coroutine and registers the fd with the Reactor.
// Each time the fd becomes ready, perform_io() runs one partial read or write, accumulating
// bytes into _buffer at _offset. When StopCondition is satisfied the coroutine is resumed.
// await_resume() returns the total number of bytes transferred.
//
// Mode         — Read or Write, selects the fd_set the Reactor watches
// StopCondition — callable(span, offset) -> bool; true means transfer is complete
// IoFunc        — callable(fd, ptr, count) -> ssize_t; wraps ::read or ::write

template<Reactor::FdMode Mode, typename StopCondition, typename IoFunc>
struct AsyncBuffer {
    // CharType is 'char' for reads, 'const char' for writes.
    using CharType = std::conditional_t<Mode == Reactor::FdMode::Read, char, const char>;

    Reactor &_reactor;
    int _fd;
    std::span<CharType> _buffer;
    std::coroutine_handle<> _handle;  // the suspended coroutine, set in await_suspend
    size_t _offset = 0;               // bytes transferred so far

    AsyncBuffer(Reactor &reactor, int fd, std::span<CharType> buffer)
        : _reactor(reactor),
          _fd(fd),
          _buffer(buffer) {}

    // NOLINTBEGIN(readability-convert-member-functions-to-static) -- co_await awaitable

    // Always suspend; even a zero-byte buffer must be consistent with the async model.
    bool await_ready() {
        return false;
    }

    // Store the coroutine handle and start the first I/O attempt via the Reactor.
    void await_suspend(std::coroutine_handle<> handle) {
        _handle = handle;
        _offset = 0;

        if (is_done()) {
            // The stop condition is already met (e.g. zero-length buffer). Resume immediately.
            _handle.resume();
            return;
        }

        post_perform_io();
    }

    // Return the total bytes transferred when the coroutine resumes.
    [[nodiscard]] size_t await_resume() const {
        return _offset;
    }

    // NOLINTEND(readability-convert-member-functions-to-static)

private:
    // Return true when the stop condition is satisfied.
    [[nodiscard]] bool is_done() const {
        return StopCondition{}(_buffer, _offset);
    }

    // Return true for errors that mean "try again later" (non-blocking fd not ready, or signal).
    [[nodiscard]] static bool should_retry(const ssize_t n) {
        return n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
    }

    // Register the fd with the Reactor so perform_io() is called when it is ready.
    void post_perform_io() {
        _reactor.post(_fd, Mode, [this](int) { perform_io(); });
    }

    // Perform one partial I/O and decide whether to continue, retry, or finish.
    void perform_io() {
        const ssize_t n = IoFunc{}(_fd, _buffer.data() + _offset, _buffer.size() - _offset);

        if (should_retry(n)) {
            // The fd is not ready. Stay registered; the Reactor will call us again.
            return;
        }

        if (n <= 0) {
            // EOF or an unrecoverable error. Stop with however many bytes arrived so far.
            remove_and_resume();
            return;
        }

        _offset += static_cast<size_t>(n);

        if (is_done()) {
            // The stop condition is satisfied. Resume the coroutine.
            remove_and_resume();
        }
        // Otherwise stay registered and wait for the next ready notification.
    }

    // Unregister the fd and resume the suspended coroutine.
    void remove_and_resume() const {
        _reactor.remove(_fd, Mode);
        _handle.resume();
    }
};

}
