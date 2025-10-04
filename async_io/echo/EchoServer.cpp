#include "EchoServer.h"
#include "reactor/Reactor.h"
#include "async/AsyncBuffer.h"
#include <iostream>

EchoServer::EchoServer(Reactor& reactor, int read_fd, int write_fd)
    : _reactor(reactor), _read_fd(read_fd), _write_fd(write_fd) {}

AsyncIoCoroutine EchoServer::run() {
    std::cout << "[Server] Started" << std::endl;
    
    char buffer[256];
    
    while (true) {
        // Read from pipe1 until newline
        size_t total = co_await AsyncReadUntil<'\n'>{_reactor, _read_fd, buffer};
        
        if (total == 0) {
            std::cout << "[Server] EOF on pipe1" << std::endl;
            co_return;
        }
        
        log_received_message(std::span<const char>(buffer, total));
        
        // Echo to pipe2
        size_t written = co_await AsyncWriteExact<>{_reactor, _write_fd,
                                                     std::span<const char>(buffer, total)};
        
        if (written < total) {
            std::cout << "[Server] ✗ ERROR: Failed to write all bytes to pipe2!" << std::endl;
            std::cout << "  Expected " << total << " bytes, wrote " << written << " bytes" << std::endl;
            break;
        }
        
        log_echo_result(written);
    }
    
    std::cout << "[Server] Finished" << std::endl;
}

void EchoServer::log_received_message(std::span<const char> data) {
    std::string_view message(data.data(), data.size());
    std::cout << "[Server] Received: " << message;
}

void EchoServer::log_echo_result(size_t bytes_written) {
    std::cout << "[Server] Echoed " << bytes_written << " bytes to pipe2" << std::endl;
}