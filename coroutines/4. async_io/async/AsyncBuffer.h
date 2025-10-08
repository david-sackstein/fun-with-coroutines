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

            if (is_done()) {
                _handle.resume();
                return;
            }

            post_perform_io();
        }

        size_t await_resume() {
            return _offset;
        }

    private:
        void post_perform_io() {
            _reactor.post(_fd, Mode, [this](int) {
                perform_io();
            });
        }

        void perform_io() {
            ssize_t n = IoFunc{}(_fd, _buffer.data() + _offset, _buffer.size() - _offset);

            if (should_retry(n)) {
                // Stay registered, reactor will notify again when ready
                return;
            }

            if (n <= 0) {
                // Stop immediately on EOF or real error
                remove_and_resume();
                return;
            }

            // Success: n > 0
            _offset += n;

            if (is_done()) {
                remove_and_resume();
            }
        }

        void remove_and_resume() {
            _reactor.remove(_fd, Mode);
            _handle.resume();
        }

        bool should_retry(ssize_t n) const {
            return n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
        }

        bool is_done() const {
            return StopCondition{}(_buffer, _offset);
        }
    };
}
