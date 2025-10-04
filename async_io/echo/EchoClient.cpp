#include "EchoClient.h"
#include "reactor/Reactor.h"
#include "async/AsyncReadBuffer.h"
#include "async/AsyncWriteBuffer.h"
#include <iostream>
#include <cstring>

EchoClient::EchoClient(Reactor& reactor, int stdin_fd, int write_fd, int read_fd)
    : _reactor(reactor), _stdin_fd(stdin_fd), _write_fd(write_fd), _read_fd(read_fd) {}

AsyncIoCoroutine EchoClient::run() {
    std::cout << "[Client] Started" << std::endl;
    
    char write_buffer[256];
    char read_buffer[256];
    
    while (true) {
        // Read from stdin
        std::cout << "[Client] Waiting for input..." << std::endl;
        ssize_t n = co_await AsyncReadBuffer{_reactor, _stdin_fd, write_buffer};
        
        if (n <= 0) {
            std::cout << "[Client] EOF on stdin" << std::endl;
            break;
        }
        
        std::string_view input(write_buffer, n);
        std::cout << "[Client] Read from stdin: " << input;
        
        // Write to pipe1
        ssize_t written = co_await f{_reactor, _write_fd,
                                                     std::span<const char>(write_buffer, n)};
        std::cout << "[Client] Wrote " << written << " bytes to pipe1" << std::endl;
        
        // Read from pipe2
        ssize_t echoed = co_await AsyncReadBuffer{_reactor, _read_fd, read_buffer};
        
        if (echoed <= 0) {
            std::cout << "[Client] EOF on pipe2" << std::endl;
            break;
        }
        
        std::string_view echo(read_buffer, echoed);
        std::cout << "[Client] Read from pipe2: " << echo;
        
        // Verify
        if (echoed == n && std::memcmp(write_buffer, read_buffer, n) == 0) {
            std::cout << "[Client] ✓ Echo verified successfully!" << std::endl;
        } else {
            std::cout << "[Client] ✗ ERROR: Echo mismatch!" << std::endl;
            std::cout << "  Expected " << n << " bytes, got " << echoed << " bytes" << std::endl;
        }
    }
    
    std::cout << "[Client] Stopping reactor" << std::endl;
    _reactor.stop();
}

