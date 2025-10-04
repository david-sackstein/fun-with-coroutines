#include "EchoServer.h"
#include "reactor/Reactor.h"
#include "async/AsyncReadBuffer.h"
#include "async/AsyncWriteExact.h"
#include <iostream>

EchoServer::EchoServer(Reactor& reactor, int read_fd, int write_fd)
    : _reactor(reactor), _read_fd(read_fd), _write_fd(write_fd) {}

AsyncIoCoroutine EchoServer::run() {
    std::cout << "[Server] Started" << std::endl;
    
    char buffer[256];
    
    while (true) {
        // Read from pipe1 (single read - we don't know size in advance)
        ssize_t n = co_await AsyncReadBuffer{_reactor, _read_fd, buffer};
        
        if (n <= 0) {
            std::cout << "[Server] EOF on pipe1" << std::endl;
            break;
        }
        
        std::string_view data(buffer, n);
        std::cout << "[Server] Received: " << data;
        
        // Echo EXACTLY n bytes to pipe2 (loop until all written)
        size_t written = co_await AsyncWriteExact{_reactor, _write_fd, 
                                                   std::span<const char>(buffer, n)};
        
        if (written < static_cast<size_t>(n)) {
            std::cout << "[Server] ✗ ERROR: Failed to write all bytes to pipe2!" << std::endl;
            std::cout << "  Expected " << n << " bytes, wrote " << written << " bytes" << std::endl;
            break;
        }
        std::cout << "[Server] Echoed " << written << " bytes to pipe2" << std::endl;
    }
    
    std::cout << "[Server] Finished" << std::endl;
}

