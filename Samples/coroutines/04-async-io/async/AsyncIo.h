#pragma once

#include "coroutines/04-async-io/async/AsyncBuffer.h"

namespace coroutines {

// I/O function wrappers passed as IoFunc template argument to AsyncBuffer.
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

// Stop condition: finish after a single I/O call regardless of how many bytes were read.
struct SingleShot {
    bool operator()(std::span<const char>, size_t) const {
        return true;
    }
};

// Stop condition: finish when the buffer is completely filled.
struct UntilFull {
    bool operator()(const std::span<const char> buffer, const size_t offset) const {
        return offset == buffer.size();
    }
};

// Stop condition: finish when the last byte read equals Delimiter (e.g. '\n').
template<char Delimiter>
struct UntilDelimiter {
    bool operator()(const std::span<const char> buffer, const size_t offset) const {
        return offset != 0 && buffer[offset - 1] == Delimiter;
    }
};

// Read one chunk from fd; co_await returns the number of bytes read.
inline auto async_read_buffer(Reactor &reactor, const int fd, const std::span<char> buffer) {
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return AsyncBuffer<Reactor::FdMode::Read, SingleShot, DefaultRead>{reactor, fd, buffer};
}

// Read from fd until the buffer is full; co_await returns the number of bytes read.
inline auto async_read_exact(Reactor &reactor, const int fd, const std::span<char> buffer) {
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return AsyncBuffer<Reactor::FdMode::Read, UntilFull, DefaultRead>{reactor, fd, buffer};
}

// Read from fd until Delimiter is received or the buffer is full; co_await returns bytes read.
template<char Delimiter>
auto async_read_until(Reactor &reactor, const int fd, const std::span<char> buffer) {
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return AsyncBuffer<Reactor::FdMode::Read, UntilDelimiter<Delimiter>, DefaultRead>{reactor, fd, buffer};
}

// Write one chunk to fd; co_await returns the number of bytes written.
inline auto async_write_buffer(Reactor &reactor, const int fd, const std::span<const char> buffer) {
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return AsyncBuffer<Reactor::FdMode::Write, SingleShot, DefaultWrite>{reactor, fd, buffer};
}

// Write the entire buffer to fd; co_await returns the number of bytes written.
inline auto async_write_exact(Reactor &reactor, const int fd, const std::span<const char> buffer) {
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return AsyncBuffer<Reactor::FdMode::Write, UntilFull, DefaultWrite>{reactor, fd, buffer};
}

}
