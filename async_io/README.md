# Async I/O with Coroutines

This directory demonstrates using C++20 coroutines with I/O multiplexing (select-based).

## Architecture

### Core Components

1. **Selector** - Event loop using `select()` for I/O multiplexing
   - Manages file descriptor handlers for read/write events
   - Dispatches handlers when fds become ready

2. **AsyncRead/AsyncWrite** - Awaitable types for I/O operations
   - Suspend coroutine execution when awaited
   - Register handler with Selector
   - Resume coroutine when I/O is ready

3. **CoroutineObject** - Coroutine return type
   - Manages coroutine lifetime
   - Starts immediately (`suspend_never`)
   - Auto-destroys on completion

## How It Works

### Coroutine Flow

```cpp
CoroutineObject read_lines(Selector& selector) {
    co_await AsyncRead{selector, STDIN_FILENO};  // Suspends here
    // ... read from stdin ...
    selector.stop();
}
```

### Execution Steps

1. **Coroutine starts** - `read_lines()` begins executing
2. **Hit co_await** - Execution suspends at `AsyncRead`
3. **Handler registered** - `AsyncRead::await_suspend()` posts handler to Selector
4. **Control returns** - Execution returns to `selector.run()`
5. **select() waits** - Selector blocks in `select()` waiting for I/O
6. **I/O ready** - `select()` returns when stdin has data
7. **Handler dispatched** - Selector invokes the registered handler
8. **Coroutine resumes** - Handler calls `coroutine_handle.resume()`
9. **Continue execution** - Code after `co_await` executes
10. **Stop selector** - `selector.stop()` exits the event loop

## Pattern Similarity to async_tasks

| async_tasks | async_io |
|-------------|----------|
| `EventLoop` | `Selector` |
| `AsyncTask` | `AsyncRead/AsyncWrite` |
| Posts tasks to queue | Registers I/O handlers |
| Waits on condition variable | Waits in `select()` |
| Resumes on worker thread | Resumes in event loop |

## Key Design Decisions

### One-Shot Handlers
The `AsyncRead/AsyncWrite` awaitables automatically remove their handlers after triggering once. This prevents spurious wakeups and matches the expected coroutine behavior.

### Synchronous Dispatch
Coroutines resume directly in `dispatch_ready()`, executing synchronously within the event loop. This is simpler than async_tasks' thread pool approach and sufficient for I/O-bound operations.

### Clean Suspension Points
Each `co_await` is an explicit suspension point where control returns to `select()`. This makes the control flow clear and predictable.

## Usage Example

```cpp
CoroutineObject my_io_task(Selector& selector, int fd) {
    // Wait for fd to be readable
    co_await AsyncRead{selector, fd};
    
    // Now it's ready - do the read
    char buffer[1024];
    ssize_t n = read(fd, buffer, sizeof(buffer));
    
    // Wait for another fd to be writable
    co_await AsyncWrite{selector, other_fd};
    
    // Write the data
    write(other_fd, buffer, n);
}
```

## Advantages Over Callbacks

**Before (callbacks):**
```cpp
selector.post(fd, Read, [](int fd) {
    read(fd, ...);
    // Nested callbacks for multiple operations...
});
```

**After (coroutines):**
```cpp
co_await AsyncRead{selector, fd};
read(fd, ...);
// Sequential code flow!
```

The coroutine version:
- ✅ Reads like synchronous code
- ✅ Avoids callback nesting
- ✅ Easy error handling with try/catch
- ✅ Natural loop constructs
- ✅ Still non-blocking and efficient

