# Non-Blocking I/O Error Handling

## Overview

With non-blocking FDs, `::read()` and `::write()` can return errors that require special handling:

1. **EAGAIN / EWOULDBLOCK** - FD not ready yet (expected, retry)
2. **EINTR** - System call interrupted by signal (expected, retry)
3. **Other errors** - Real errors (EOF, broken pipe, etc.)

## Implementation

### EAGAIN / EWOULDBLOCK Handling

When `::read()` or `::write()` returns `-1` with `errno == EAGAIN` or `errno == EWOULDBLOCK`:

```cpp
if (errno == EAGAIN || errno == EWOULDBLOCK) {
    // FD not ready yet, wait for it to become ready again
    read_next();  // Re-post to reactor and wait
    return;
}
```

**Why this happens:**
- FD is non-blocking but not ready (rare with `select()`)
- Between `select()` reporting "ready" and our `::read()`, state changed
- Could happen with edge-triggered notifications or race conditions

**Solution:**
- Call `read_next()` again to re-post to reactor
- Reactor will use `select()` to wait for FD to become ready
- Then retry the operation

### EINTR Handling

When system call is interrupted by a signal:

```cpp
if (errno == EINTR) {
    // Interrupted by signal, retry immediately
    reactor.post(fd, Reactor::FdMode::Read, [this](int) {
        reactor.remove(fd, Reactor::FdMode::Read);
        read_next();  // Retry the operation
    });
    return;
}
```

**Why this happens:**
- Signal delivered during `::read()` or `::write()`
- System call interrupted before completion
- Common with timers, `SIGCHLD`, etc.

**Solution:**
- Re-post to reactor immediately (no need to wait)
- Retry the same operation
- POSIX says we should retry after `EINTR`

### Real Errors

All other errors (including `n == 0` for EOF):

```cpp
if (n < 0) {
    // After checking EAGAIN/EWOULDBLOCK/EINTR above
    // This is a real error
    handle.resume();
    return;
}

if (n == 0) {
    // EOF
    handle.resume();
    return;
}
```

**Errors include:**
- `EBADF` - Bad file descriptor
- `EPIPE` - Broken pipe (write to closed reader)
- `EIO` - I/O error
- `ECONNRESET` - Connection reset (sockets)

**Solution:**
- Resume coroutine with current `offset`
- Caller checks return value and handles error

## Error Flow Diagram

```
::read(fd, buffer, size)
         |
         v
    n < 0 ?  ──Yes──> errno == EAGAIN/EWOULDBLOCK ? ──Yes──> read_next() (wait)
         |                      |
         No                     No
         |                      v
         v                 errno == EINTR ? ──Yes──> reactor.post() → read_next()
    n == 0 ?                   |
         |                     No
         |                     v
        Yes              Real error
         |                     |
         v                     v
        EOF              Resume with offset
         |                     |
         v                     v
    Resume with offset    Caller handles error
```

## Why Both EAGAIN and EWOULDBLOCK?

**Historical reasons:**
- POSIX says they should have the same value
- Some systems define them separately
- `errno == EAGAIN || errno == EWOULDBLOCK` covers both

**On most systems:**
```c
#define EAGAIN       35
#define EWOULDBLOCK  EAGAIN  // Same value
```

But defensive programming checks both explicitly.

## Testing

To trigger these errors:

### EAGAIN/EWOULDBLOCK
- Rare with `select()` (FD should be ready when we read)
- Can happen with level-triggered I/O multiplexing
- May occur in high-load scenarios

### EINTR
```bash
# Send signal to process during I/O
kill -USR1 <pid>
```

## Code Locations

- **`AsyncReadBuffer.h`** - Read error handling (lines 43-69)
- **`AsyncWriteBuffer.h`** - Write error handling (lines 43-69)

## Summary

| Error | Meaning | Action |
|-------|---------|--------|
| `EAGAIN` / `EWOULDBLOCK` | FD not ready | Re-post to reactor, wait |
| `EINTR` | Interrupted by signal | Re-post immediately, retry |
| `n == 0` | EOF (read only) | Resume, caller handles |
| Other | Real error | Resume, caller handles |

**Result:** Robust handling of all non-blocking I/O error cases! ✅

