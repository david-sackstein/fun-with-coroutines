#include "EchoClient.h"
#include "reactor/Reactor.h"
#include "async/AsyncBuffer.h"
#include <print>
#include <cstring>

EchoClient::EchoClient(Reactor& reactor, int stdin_fd, int write_fd, int read_fd)
    : _reactor(reactor), _stdin_fd(stdin_fd), _write_fd(write_fd), _read_fd(read_fd) {}

AsyncIoCoroutine EchoClient::run() {
    std::print("[Client] Started\n");
    
    char write_buffer[256];
    char read_buffer[256];
    
    while (true) {
        std::print("[Client] Waiting for input...\n");
        size_t total = co_await async_read_until<'\n'>(_reactor, _stdin_fd, write_buffer);
        
        if (total == 0) {
            std::print("[Client] EOF on stdin\n");
            _reactor.stop();
            co_return;
        }

        log_input(write_buffer, total);
        
        size_t written = co_await async_write_exact(_reactor, _write_fd, {write_buffer, total});
        if (!check_write_complete(total, written)) {
            break;
        }
        
        size_t echoed = co_await async_read_exact(_reactor, _read_fd, {read_buffer, total});
        if (!check_read_complete(total, echoed)) {
            break;
        }
        
        if (!verify_and_log_echo(write_buffer, total, read_buffer, echoed)) {
            break;
        }
    }
    
    std::print("[Client] Stopping reactor\n");
    _reactor.stop();
}

void EchoClient::log_input(const char *data, size_t size) {
    std::print("[Client] Read from stdin: {}", std::string_view(data, size));
}

bool EchoClient::check_write_complete(size_t expected, size_t actual) {
    if (actual < expected) {
        std::print("[Client] ✗ ERROR: Failed to write all bytes to pipe1!\n");
        std::print("  Expected {} bytes, wrote {} bytes\n", expected, actual);
        return false;
    }
    std::print("[Client] Wrote {} bytes to pipe1\n", actual);
    return true;
}

bool EchoClient::check_read_complete(size_t expected, size_t actual) {
    if (actual < expected) {
        std::print("[Client] ✗ ERROR: Failed to read all bytes from pipe2!\n");
        std::print("  Expected {} bytes, got {} bytes\n", expected, actual);
        return false;
    }
    return true;
}

bool EchoClient::verify_and_log_echo(const char *sent, size_t sent_size, 
                                     const char *received, size_t received_size) {
    if (std::memcmp(sent, received, sent_size) == 0) {
        std::print("[Client] Read from pipe2: {}", std::string_view(received, received_size));
        std::print("[Client] ✓ Echo verified successfully!\n");
        return true;
    }
    std::print("[Client] ✗ ERROR: Echo mismatch!\n");
    return false;
}