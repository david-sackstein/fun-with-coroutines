#include "EchoServer.h"
#include "reactor/Reactor.h"
#include "async/AsyncBuffer.h"
#include <print>

EchoServer::EchoServer(Reactor& reactor, int read_fd, int write_fd)
    : _reactor(reactor), _read_fd(read_fd), _write_fd(write_fd) {}

AsyncIoCoroutine EchoServer::run() {
    std::print("[Server] Started\n");
    
    char buffer[256];
    
    while (true) {
        size_t total = co_await async_read_until<'\n'>(_reactor, _read_fd, buffer);
        
        if (total == 0) {
            std::print("[Server] EOF on pipe1\n");
            co_return;
        }
        
        log_received_message(std::span<const char>(buffer, total));
        
        size_t written = co_await async_write_exact(_reactor, _write_fd,
                                                     std::span<const char>(buffer, total));
        if (!check_write_complete(total, written)) {
            break;
        }
    }
    
    std::print("[Server] Finished\n");
}

void EchoServer::log_received_message(std::span<const char> data) {
    std::string_view message(data.data(), data.size());
    std::print("[Server] Received: {}", message);
}

bool EchoServer::check_write_complete(size_t expected, size_t actual) {
    if (actual < expected) {
        std::print("[Server] ✗ ERROR: Failed to write all bytes to pipe2!\n");
        std::print("  Expected {} bytes, wrote {} bytes\n", expected, actual);
        return false;
    }
    std::print("[Server] Echoed {} bytes to pipe2\n", actual);
    return true;
}