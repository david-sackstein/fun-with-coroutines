#pragma once

// Callback-based async I/O helpers for the no-coroutines version.
// Compare with the coroutines version: the same partial-read/write loop is
// expressed here as a self-re-posting Reactor handler instead of a co_await expression.
//
// post_read and post_write register a handler with the Reactor. Each time the fd is
// ready, the handler does one partial operation and re-registers itself until
// the stop condition is met, then calls on_complete with the total bytes transferred.

#include "common/reactor/Reactor.h"

#include <cstddef>
#include <functional>

namespace no_coroutines {

// Callback invoked when an async I/O operation completes; receives the byte count.
using IoComplete = std::function<void(size_t bytes_done)>;
// Predicate consulted after each partial read; returns true when reading should stop.
using ReadStopWhen = std::function<bool(const char *buffer, size_t offset, size_t limit)>;

// Stop reading after a newline is received or the buffer is full.
[[nodiscard]] ReadStopWhen stop_at_newline_or_full();

// Stop reading when exactly limit bytes have been received.
[[nodiscard]] ReadStopWhen stop_at_exact_bytes();

// Register a non-blocking read on fd; call on_complete with total bytes when stop_when is satisfied.
void post_read(
    Reactor &reactor,
    int fd,
    char *buffer,
    size_t limit,
    ReadStopWhen stop_when,
    IoComplete on_complete);

// Register a non-blocking write of total bytes from buffer on fd; call on_complete when done.
void post_write(
    Reactor &reactor,
    int fd,
    const char *buffer,
    size_t total,
    IoComplete on_complete);

}
