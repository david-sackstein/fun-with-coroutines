#include "EchoServer.h"
#include "reactor/Reactor.h"
#include "async/AsyncReadBuffer.h"
#include "async/AsyncWriteBuffer.h"
#include <iostream>

EchoServer::EchoServer(Reactor& reactor, int read_fd, int write_fd)
    : _reactor(reactor), _read_fd(read_fd), _write_fd(write_fd) {}

AsyncIoCoroutine EchoServer::run() {
    std::cout << "[Server] Started" << std::endl;
    
    char buffer[256];
    size_t total = 0;
    
    while (true) {
        // Read until we get a complete line (ends with \n)
        while (total < sizeof(buffer) - 1) {
            ssize_t n = co_await AsyncReadBuffer<>{_reactor, _read_fd, 
                                                    std::span<char>(buffer + total, sizeof(buffer) - total)};
            
            if (n <= 0) {
                std::cout << "[Server] EOF on pipe1" << std::endl;
                co_return;
            }
            
            total += n;
            
            // Check if the last byte is a newline
            if (buffer[total - 1] == '\n') {
                break;  // Complete line received
            }
        }
        
        std::string_view data(buffer, total);
        std::cout << "[Server] Received: " << data;
        
        // Echo EXACTLY total bytes to pipe2 (loop until all written)
        size_t written = co_await AsyncWriteBuffer<true>{_reactor, _write_fd,
                                                          std::span<const char>(buffer, total)};
        
        if (written < total) {
            std::cout << "[Server] ✗ ERROR: Failed to write all bytes to pipe2!" << std::endl;
            std::cout << "  Expected " << total << " bytes, wrote " << written << " bytes" << std::endl;
            break;
        }
        
        std::cout << "[Server] Echoed " << written << " bytes to pipe2" << std::endl;
        total = 0;  // Reset for next message
    }
    
    std::cout << "[Server] Finished" << std::endl;
}
