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
        // Read until newline (using stop condition!)
        size_t total = co_await AsyncReadUntil<'\n'>{_reactor, _read_fd, buffer};
        
        if (total == 0) {
            std::cout << "[Server] EOF on pipe1" << std::endl;
            co_return;
        }
        
        std::string_view data(buffer, total);
        std::cout << "[Server] Received: " << data;
        
        // Echo EXACTLY total bytes to pipe2 (loop until all written)
        size_t written = co_await AsyncWriteExact<>{_reactor, _write_fd,
                                                     std::span<const char>(buffer, total)};
        
        if (written < total) {
            std::cout << "[Server] ✗ ERROR: Failed to write all bytes to pipe2!" << std::endl;
            std::cout << "  Expected " << total << " bytes, wrote " << written << " bytes" << std::endl;
            break;
        }
        
        std::cout << "[Server] Echoed " << written << " bytes to pipe2" << std::endl;
    }
    
    std::cout << "[Server] Finished" << std::endl;
}