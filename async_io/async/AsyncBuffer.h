#pragma once

#include "reactor/Reactor.h"

#include <coroutine>
#include <span>
#include <unistd.h>
#include <cerrno>
#include <type_traits>

// ============================================================================
// AsyncBuffer - Generic Async I/O with Coroutines
// ============================================================================
//
// A zero-cost abstraction for non-blocking I/O operations with three axes
// of generalization:
//
// 1. Mode: Read vs Write (compile-time via template parameter)
// 2. StopCondition: When to stop (single-shot, until full, or custom)
// 3. IoFunc: How to perform I/O (::read/::write or custom like ::recv/::send)
//
// Design Philosophy:
// - Zero overhead: [[no_unique_address]] for stateless functors
// - Compile-time specialization: if constexpr eliminates unused code
// - Non-blocking: All FDs must be set to O_NONBLOCK
// - Error handling: EAGAIN/EWOULDBLOCK/EINTR handled transparently
//
// Usage:
//   co_await AsyncReadBuffer<>{reactor, fd, buffer};        // Read once
//   co_await AsyncReadExact<>{reactor, fd, buffer};         // Read until full
//   co_await AsyncReadUntil<'\n'>{reactor, fd, buffer};     // Read until '\n'
//
// Note on AsyncReadUntil:
//   Returns all bytes up to and including the LAST delimiter in the read.
//   If multiple delimiters are present (e.g., "Line1\nLine2\n"), all bytes
//   are returned. For true line-by-line streaming with leftover data handling,
//   use a BufferedReader (not included in this file).
//
// ============================================================================

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

// ============================================================================
// Stop Conditions
// ============================================================================
//
// Stop conditions determine when to stop reading/writing. They receive:
//   - buffer: The full buffer being filled/emptied
//   - offset: Current number of bytes processed
//
// Return value:
//   - true: Need more data, continue I/O
//   - false: Done, stop I/O
//
// ============================================================================

// SingleShot: Stop after one I/O operation (default)
struct SingleShot {
    bool operator()(std::span<const char>, size_t) const {
        return false;  // Always done after one I/O
    }
};

// UntilFull: Continue until buffer is completely filled
struct UntilFull {
    bool operator()(std::span<const char> buffer, size_t offset) const {
        return offset < buffer.size();  // Continue until buffer full
    }
};

// UntilDelimiter: Continue until delimiter character is found
// Note: Checks if LAST byte is the delimiter, so if multiple delimiters
//       are present in one read, all data up to the last one is returned.
template<char Delimiter>
struct UntilDelimiter {
    bool operator()(std::span<const char> buffer, size_t offset) const {
        if (offset == 0) return true;  // Need at least one byte
        return buffer[offset - 1] != Delimiter;  // Stop when delimiter found
    }
};

// ============================================================================
// AsyncBuffer - Main Template
// ============================================================================

template<Reactor::FdMode Mode,
         typename StopCondition = SingleShot,
         typename IoFunc = std::conditional_t<Mode == Reactor::FdMode::Read, DefaultRead, DefaultWrite>>
struct AsyncBuffer {
    using CharType = std::conditional_t<Mode == Reactor::FdMode::Read, char, const char>;
    
    Reactor& reactor;
    int fd;
    std::span<CharType> buffer;
    [[no_unique_address]] StopCondition stop_condition;  // Zero-size for stateless
    [[no_unique_address]] IoFunc io_func;                // Zero-size for stateless
    
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
    // Initial entry point - checks if already done before posting
    void wait_and_io() {
        if (!needs_more_data()) {
            handle.resume();
            return;
        }
        post_io();
    }
    
    // Post I/O operation to reactor (no checks, caller verified need)
    void post_io() {
        reactor.post(fd, Mode, [this](int) {
            reactor.remove(fd, Mode);
            perform_io();
        });
    }
    
    // Perform I/O and handle the result
    // Error Handling:
    //   - EAGAIN/EWOULDBLOCK: FD not ready (rare with select), re-post and wait
    //   - EINTR: Signal interrupted, re-post and retry
    //   - n <= 0: EOF or real error, stop with current offset
    //   - n > 0: Success, update offset and check if more data needed
    void perform_io() {
        ssize_t n = do_io();
        
        if (should_retry(n)) {
            // Transient error: FD not ready or interrupted by signal
            // Re-post to reactor and wait for FD to become ready
            post_io();
            return;
        }
        
        if (should_stop(n)) {
            // EOF or permanent error - stop with whatever we've read so far
            handle.resume();
            return;
        }
        
        // Success: n > 0
        offset += n;
        
        if (needs_more_data()) {
            // Not done yet, continue I/O
            post_io();
        } else {
            // Done!
            handle.resume();
        }
    }
    
    // Perform the actual I/O syscall
    ssize_t do_io() {
        return io_func(fd, buffer.data() + offset, buffer.size() - offset);
    }
    
    // Check if we should retry the I/O operation
    // Returns true for:
    //   - EAGAIN: FD not ready yet (rare with select(), but can happen)
    //   - EWOULDBLOCK: Same as EAGAIN on most systems (checked for portability)
    //   - EINTR: System call interrupted by signal (retry per POSIX)
    //
    // Why EAGAIN/EWOULDBLOCK can happen:
    //   - Race condition between select() reporting ready and our read/write
    //   - Edge-triggered event notification
    //   - High-load scenarios
    //
    // Why EINTR happens:
    //   - Signal delivered during system call (SIGCHLD, timers, etc.)
    //   - POSIX requires applications to retry after EINTR
    //
    // Historical note on EAGAIN vs EWOULDBLOCK:
    //   - POSIX says they should have the same value
    //   - Some systems define them separately
    //   - Defensive programming checks both
    bool should_retry(ssize_t n) const {
        return n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
    }
    
    // Check if we should stop due to error or EOF
    // Returns true for:
    //   - n == 0: EOF (for reads) or unusual condition (for writes)
    //   - n < 0: Real error (EBADF, EPIPE, EIO, ECONNRESET, etc.)
    //
    // When this returns true, we resume with current offset.
    // Caller should check the return value to detect errors:
    //   - For exact reads: (returned < expected) indicates error/EOF
    //   - For single-shot: caller decides if return value is acceptable
    bool should_stop(ssize_t n) const {
        return n <= 0;
    }
    
    // Check if more data is needed
    // Delegates to stop_condition which sees:
    //   - buffer: The full buffer (not just filled portion)
    //   - offset: Current number of bytes processed
    bool needs_more_data() const {
        return stop_condition(buffer, offset);
    }
    
    std::coroutine_handle<> handle;
    size_t offset = 0;
};

// Convenient Type Aliases

// Read operations
template<typename IoFunc = DefaultRead>
using AsyncReadBuffer = AsyncBuffer<Reactor::FdMode::Read, SingleShot, IoFunc>;

template<typename IoFunc = DefaultRead>
using AsyncReadExact = AsyncBuffer<Reactor::FdMode::Read, UntilFull, IoFunc>;

template<char Delimiter, typename IoFunc = DefaultRead>
using AsyncReadUntil = AsyncBuffer<Reactor::FdMode::Read, UntilDelimiter<Delimiter>, IoFunc>;

// Write operations
template<typename IoFunc = DefaultWrite>
using AsyncWriteBuffer = AsyncBuffer<Reactor::FdMode::Write, SingleShot, IoFunc>;

template<typename IoFunc = DefaultWrite>
using AsyncWriteExact = AsyncBuffer<Reactor::FdMode::Write, UntilFull, IoFunc>;