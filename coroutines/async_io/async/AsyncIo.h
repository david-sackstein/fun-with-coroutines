#pragma once

#include "coroutines/async_io/async/AsyncBuffer.h"

namespace coroutines {
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
        bool operator()(std::span<const char>, size_t) const { return true; }
    };

    struct UntilFull {
        bool operator()(std::span<const char> buffer, size_t offset) const {
            return offset == buffer.size();
        }
    };

    template<char Delimiter>
    struct UntilDelimiter {
        bool operator()(std::span<const char> buffer, size_t offset) const {
            return offset != 0 && buffer[offset - 1] == Delimiter;
        }
    };

// ============================================================================
// Factory Functions - Public API
// ============================================================================

#pragma clang diagnostic push
// Suppress a false linter alarm. The buffer argument points to a variable allocated in the coroutine frame so it safe.
#pragma ide diagnostic ignored "LocalValueEscapesScope"

    inline auto async_read_buffer(Reactor &reactor, int fd, std::span<char> buffer) {
        return AsyncBuffer<Reactor::FdMode::Read, SingleShot, DefaultRead>{reactor, fd, buffer};
    }

    inline auto async_read_exact(Reactor &reactor, int fd, std::span<char> buffer) {
        return AsyncBuffer<Reactor::FdMode::Read, UntilFull, DefaultRead>{reactor, fd, buffer};
    }

    template<char Delimiter>
    inline auto async_read_until(Reactor &reactor, int fd, std::span<char> buffer) {
        return AsyncBuffer<Reactor::FdMode::Read, UntilDelimiter<Delimiter>, DefaultRead>{reactor, fd, buffer};
    }

    inline auto async_write_buffer(Reactor &reactor, int fd, std::span<const char> buffer) {
        return AsyncBuffer<Reactor::FdMode::Write, SingleShot, DefaultWrite>{reactor, fd, buffer};
    }

    inline auto async_write_exact(Reactor &reactor, int fd, std::span<const char> buffer) {
        return AsyncBuffer<Reactor::FdMode::Write, UntilFull, DefaultWrite>{reactor, fd, buffer};
    }
#pragma clang diagnostic pop
}