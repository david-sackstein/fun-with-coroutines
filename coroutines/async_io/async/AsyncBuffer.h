#pragma once

#include "common/reactor/Reactor.h"

#include <coroutine>
#include <span>
#include <cerrno>

namespace coroutines {
// ============================================================================
// AsyncBuffer - Internal awaitable implementation
// ============================================================================

    template<Reactor::FdMode Mode, typename StopCondition, typename IoFunc>
    struct AsyncBuffer {
        using CharType = std::conditional_t<Mode == Reactor::FdMode::Read, char, const char>;

        Reactor &_reactor;
        int _fd;
        std::span<CharType> _buffer;
        std::coroutine_handle<> _handle;
        size_t _offset = 0;

        AsyncBuffer(Reactor &reactor, int fd, std::span<CharType> buffer)
            : _reactor(reactor),
              _fd(fd),
              _buffer(buffer) {}

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
            ssize_t n = IoFunc{}(_fd, _buffer.data() + _offset, _buffer.size() - _offset);

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
            return StopCondition{}(_buffer, _offset);
        }
    };
}
