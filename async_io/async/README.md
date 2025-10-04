# AsyncBuffer - Consolidated I/O Template

## Design

Single template for both read and write operations:

```cpp
template<Reactor::FdMode Mode, bool LoopUntilComplete = false>
struct AsyncBuffer {
    // Mode: Reactor::FdMode::Read or Reactor::FdMode::Write
    // LoopUntilComplete: false = single-shot, true = loop until complete
};
```

## Key Features

### 1. Compile-Time I/O Selection
```cpp
ssize_t do_io() {
    if constexpr (Mode == Reactor::FdMode::Read) {
        return ::read(fd, buffer.data() + offset, buffer.size() - offset);
    } else {
        return ::write(fd, buffer.data() + offset, buffer.size() - offset);
    }
}
```

### 2. Automatic Type Deduction
```cpp
using CharType = std::conditional_t<Mode == Reactor::FdMode::Read, char, const char>;
std::span<CharType> buffer;  // char for Read, const char for Write
```

### 3. Convenient Aliases
```cpp
template<bool Loop = false>
using AsyncReadBuffer = AsyncBuffer<Reactor::FdMode::Read, Loop>;

template<bool Loop = false>
using AsyncWriteBuffer = AsyncBuffer<Reactor::FdMode::Write, Loop>;
```

## Usage

### Single-Shot Operations
```cpp
// Read once
ssize_t n = co_await AsyncReadBuffer<>{reactor, fd, buffer};

// Write once
ssize_t n = co_await AsyncWriteBuffer<>{reactor, fd, buffer};
```

### Exact Operations (Loop Until Complete)
```cpp
// Read exactly buffer.size() bytes
size_t n = co_await AsyncReadBuffer<true>{reactor, fd, buffer};

// Write exactly buffer.size() bytes
size_t n = co_await AsyncWriteBuffer<true>{reactor, fd, buffer};
```

### Direct Template Usage
```cpp
// Also valid:
co_await AsyncBuffer<Reactor::FdMode::Read, false>{...};
co_await AsyncBuffer<Reactor::FdMode::Write, true>{...};
```

## Implementation Structure

### Method Breakdown

1. **`wait_and_io()`** - Initial check + post
   - Checks if already complete (edge case)
   - Calls `post_io()`

2. **`post_io()`** - Register with reactor
   - Posts to reactor with appropriate mode
   - Sets up callback to `perform_io()`

3. **`perform_io()`** - Main I/O logic
   - Calls `do_io()` (read or write)
   - Handles errors via `should_retry()` and `should_stop()`
   - Updates offset on success
   - Decides next action via `needs_more_data()`

4. **`do_io()`** - Compile-time I/O selection
   - `if constexpr (Mode == Read)` → `::read()`
   - `if constexpr (Mode == Write)` → `::write()`

### Control Flow
```
await_suspend()
    ↓
wait_and_io()  (checks if done)
    ↓
post_io()  (register with reactor)
    ↓
[reactor fires callback]
    ↓
perform_io()  (do I/O)
    ↓
do_io()  (::read or ::write)
    ↓
Error? → should_retry() → post_io() (retry)
EOF?   → should_stop()  → resume
Success + needs_more_data() → post_io() (continue)
Success + done → resume
```

## Error Handling

All errors handled consistently:

| Condition | Action |
|-----------|--------|
| `EAGAIN/EWOULDBLOCK/EINTR` | `post_io()` - retry |
| `n <= 0` (EOF/error) | `resume` - stop |
| `n > 0` success | Continue or resume based on `LoopUntilComplete` |

## Benefits of Consolidation

### Before
```
AsyncReadBuffer.h   →  90 lines
AsyncWriteBuffer.h  →  90 lines
──────────────────────────────
Total: 180 lines, duplicated logic
```

### After
```
AsyncBuffer.h       →  100 lines
──────────────────────────────
Total: 100 lines, single implementation
```

**Savings: 45% less code, zero duplication!**

### Additional Benefits

1. **Single Source of Truth**: All I/O logic in one place
2. **Easier Maintenance**: Fix once, applies to both read and write
3. **Type Safety**: `std::conditional_t` ensures correct buffer types
4. **Zero Overhead**: `if constexpr` eliminated at compile time
5. **Clear Separation**: 
   - `wait_and_io()` - guard + post
   - `post_io()` - just post
   - `perform_io()` - I/O logic
   - `do_io()` - actual syscall

## Line-Based Protocol

All messages end with `\n`. To read a complete line:

```cpp
char buffer[256];
size_t total = 0;

while (total < sizeof(buffer) - 1) {
    ssize_t n = co_await AsyncReadBuffer<>{reactor, fd, 
                                            std::span<char>(buffer + total, ...)};
    if (n <= 0) break;
    total += n;
    if (buffer[total - 1] == '\n') break;  // Complete line
}
```

## Non-Blocking I/O

All FDs set to `O_NONBLOCK`:
- `::read()`/`::write()` return immediately
- Reactor (`select()`) handles waiting
- EAGAIN/EWOULDBLOCK handled transparently

## Summary

**AsyncBuffer** is a unified template providing:
- Single-shot or looping I/O
- Read or write operations
- Proper error handling
- Non-blocking semantics
- Clean, maintainable code

**One template to rule them all!** ✨
