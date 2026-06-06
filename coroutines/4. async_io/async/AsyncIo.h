#pragma once

#include "coroutines/4. async_io/async/AsyncBuffer.h"

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
        bool operator()(const std::span<const char> buffer, const size_t offset) const {
            return offset == buffer.size();
        }
    };

    template<char Delimiter>
    struct UntilDelimiter {
        bool operator()(const std::span<const char> buffer, const size_t offset) const {
            return offset != 0 && buffer[offset - 1] == Delimiter;
        }
    };

    // ============================================================================
    // Factory Functions - Public API
    // ============================================================================

    inline auto async_read_buffer(Reactor &reactor, const int fd, const std::span<char> buffer) {
        // ReSharper disable once CppDFALocalValueEscapesFunction
        return AsyncBuffer<Reactor::FdMode::Read, SingleShot, DefaultRead>{reactor, fd, buffer};
    }

    inline auto async_read_exact(Reactor &reactor, const int fd, const std::span<char> buffer) {
        // ReSharper disable once CppDFALocalValueEscapesFunction
        return AsyncBuffer<Reactor::FdMode::Read, UntilFull, DefaultRead>{reactor, fd, buffer};
    }

    template<char Delimiter>
    auto async_read_until(Reactor &reactor, const int fd, const std::span<char> buffer) {
        // ReSharper disable once CppDFALocalValueEscapesFunction
        return AsyncBuffer<Reactor::FdMode::Read, UntilDelimiter<Delimiter>, DefaultRead>{reactor, fd, buffer};
    }

    inline auto async_write_buffer(Reactor &reactor, const int fd, const std::span<const char> buffer) {
        // ReSharper disable once CppDFALocalValueEscapesFunction
        return AsyncBuffer<Reactor::FdMode::Write, SingleShot, DefaultWrite>{reactor, fd, buffer};
    }

    inline auto async_write_exact(Reactor &reactor, const int fd, const std::span<const char> buffer) {
        // ReSharper disable once CppDFALocalValueEscapesFunction
        return AsyncBuffer<Reactor::FdMode::Write, UntilFull, DefaultWrite>{reactor, fd, buffer};
    }
}
