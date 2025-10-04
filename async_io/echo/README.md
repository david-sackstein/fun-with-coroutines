# Echo Server Demo

A demonstration of coroutine-based I/O using pipes to implement an echo server pattern.

## Architecture

```
┌─────────────┐         ┌─────────────┐
│ EchoClient  │         │ EchoServer  │
├─────────────┤         ├─────────────┤
│             │         │             │
│   stdin     │         │             │
│     ↓       │         │             │
│  [read]     │         │             │
│     ↓       │         │             │
│  [write]────┼────────→│  [read]     │
│             │  pipe1  │     ↓       │
│             │         │  [write]────┼───┐
│             │         │             │   │
│  [read] ←───┼─────────┼─────────────┘   │
│     ↓       │  pipe2                    │
│  [verify]   │                           │
│             │                           │
└─────────────┘                           │
```

## Classes

### Pipe
**File:** `Pipe.h` / `Pipe.cpp`

RAII wrapper for UNIX pipes:
- Creates pipe on construction
- Closes both ends on destruction
- Non-copyable
- Provides `read_fd()` and `write_fd()` accessors

```cpp
Pipe pipe;
int r = pipe.read_fd();   // Get read end
int w = pipe.write_fd();  // Get write end
```

### EchoClient
**File:** `EchoClient.h` / `EchoClient.cpp`

Reads from stdin, sends to server, verifies echo:

**Flow:**
1. `AsyncReadBuffer` from stdin → get input
2. `AsyncWriteBuffer` to pipe1 → send to server
3. `AsyncReadBuffer` from pipe2 → receive echo
4. Verify received data matches sent data
5. Print success/error message

**Coroutine:** Returns `AsyncIoCoroutine` which runs in the reactor loop.

### EchoServer
**File:** `EchoServer.h` / `EchoServer.cpp`

Echoes data from one pipe to another:

**Flow:**
1. `AsyncReadBuffer` from pipe1 → receive from client
2. `AsyncWriteBuffer` to pipe2 → echo back to client
3. Repeat until EOF

**Coroutine:** Returns `AsyncIoCoroutine` which runs in the reactor loop.

## Data Flow

1. **User types input** → stdin
2. **Client reads** from stdin using `co_await AsyncReadBuffer`
3. **Client writes** to pipe1 using `co_await AsyncWriteBuffer`
4. **Server reads** from pipe1 using `co_await AsyncReadBuffer`
5. **Server writes** to pipe2 using `co_await AsyncWriteBuffer`
6. **Client reads** from pipe2 using `co_await AsyncReadBuffer`
7. **Client verifies** echo matches original input

## Coroutine Cooperation

Both coroutines run **concurrently** in a single-threaded reactor:

- When Client suspends at `co_await AsyncReadBuffer` (stdin), control returns to reactor
- Reactor dispatches Server's pipe1 read handler when data arrives
- Server suspends at `co_await AsyncWriteBuffer` (pipe2), control returns to reactor
- Reactor dispatches Client's pipe2 read handler when data arrives
- Client resumes, verifies, and loops

**Key point:** Control bounces between coroutines via the reactor's `select()` loop.

## Object-Oriented Design Principles

### Single Responsibility
- `Pipe`: Manages pipe lifecycle
- `EchoClient`: Implements client logic
- `EchoServer`: Implements server logic
- `Reactor`: Manages I/O multiplexing

### Dependency Injection
```cpp
EchoClient client(reactor, stdin_fd, write_fd, read_fd);
EchoServer server(reactor, read_fd, write_fd);
```
All dependencies passed via constructor.

### RAII
- `Pipe` closes file descriptors automatically
- No manual resource management needed

### Separation of Concerns
- Business logic (echo verification) in `EchoClient`
- I/O mechanics delegated to `AsyncReadBuffer`/`AsyncWriteBuffer`
- Event loop mechanics delegated to `Reactor`

## Running the Demo

```bash
./fun_with_coroutines
# Type messages, press Enter
# See them echoed and verified
# Press Ctrl+D to exit
```

**Example output:**
```
[Client] Read from stdin: Hello
[Client] Wrote 6 bytes to pipe1
[Server] Received: Hello
[Server] Echoed 6 bytes to pipe2
[Client] Read from pipe2: Hello
[Client] ✓ Echo verified successfully!
```

## Error Detection

If the echo doesn't match (hypothetically), the client detects it:

```cpp
if (echoed == n && std::memcmp(write_buffer, read_buffer, n) == 0) {
    std::cout << "[Client] ✓ Echo verified successfully!" << std::endl;
} else {
    std::cout << "[Client] ✗ ERROR: Echo mismatch!" << std::endl;
}
```

## Benefits of Coroutine Approach

### vs Callbacks
```cpp
// Callback hell:
read_stdin([](data) {
    write_pipe1(data, []() {
        read_pipe2([](echo) {
            verify(echo);
        });
    });
});

// Coroutines: sequential code!
auto data = co_await read_stdin();
co_await write_pipe1(data);
auto echo = co_await read_pipe2();
verify(echo);
```

### vs Threads
- ✅ No synchronization needed
- ✅ No race conditions
- ✅ Lightweight (single-threaded)
- ✅ Deterministic scheduling

### vs Polling
- ✅ No busy-waiting
- ✅ CPU efficient
- ✅ Scales to many file descriptors

