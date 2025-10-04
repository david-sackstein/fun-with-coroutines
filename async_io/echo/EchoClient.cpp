#include "EchoClient.h"
#include "reactor/Reactor.h"
#include "async/AsyncReadBuffer.h"
#include "async/AsyncWriteExact.h"
#include "async/AsyncReadExact.h"
#include <iostream>
#include <cstring>

EchoClient::EchoClient(Reactor& reactor, int stdin_fd, int write_fd, int read_fd)
    : _reactor(reactor), _stdin_fd(stdin_fd), _write_fd(write_fd), _read_fd(read_fd) {}

AsyncIoCoroutine EchoClient::run() {
    std::cout << "[Client] Started" << std::endl;
    
    char write_buffer[256];
    char read_buffer[256];
    
    while (true) {
        // Read from stdin (single read - we don't know size in advance)
        std::cout << "[Client] Waiting for input..." << std::endl;
        ssize_t n = co_await AsyncReadBuffer{_reactor, _stdin_fd, write_buffer};
        
        if (n <= 0) {
            std::cout << "[Client] EOF on stdin" << std::endl;
            break;
        }
        
        std::string_view input(write_buffer, n);
        std::cout << "[Client] Read from stdin: " << input;
        
        // Write EXACTLY n bytes to pipe1 (loop until all written)
        size_t written = co_await AsyncWriteExact{_reactor, _write_fd, 
                                                   std::span<const char>(write_buffer, n)};
        
        if (written < static_cast<size_t>(n)) {
            std::cout << "[Client] ✗ ERROR: Failed to write all bytes to pipe1!" << std::endl;
            std::cout << "  Expected " << n << " bytes, wrote " << written << " bytes" << std::endl;
            break;
        }
        std::cout << "[Client] Wrote " << written << " bytes to pipe1" << std::endl;
        
        // Read EXACTLY n bytes from pipe2 (loop until all read)
        size_t echoed = co_await AsyncReadExact{_reactor, _read_fd, 
                                                 std::span<char>(read_buffer, n)};
        
        if (echoed < static_cast<size_t>(n)) {
            std::cout << "[Client] ✗ ERROR: Failed to read all bytes from pipe2!" << std::endl;
            std::cout << "  Expected " << n << " bytes, got " << echoed << " bytes" << std::endl;
            break;
        }
        
        std::string_view echo(read_buffer, echoed);
        std::cout << "[Client] Read from pipe2: " << echo;
        
        // Verify (now guaranteed to have correct lengths)
        if (std::memcmp(write_buffer, read_buffer, n) == 0) {
            std::cout << "[Client] ✓ Echo verified successfully!" << std::endl;
        } else {
            std::cout << "[Client] ✗ ERROR: Echo content mismatch!" << std::endl;
        }
    }
    
    std::cout << "[Client] Stopping reactor" << std::endl;
    _reactor.stop();
}
