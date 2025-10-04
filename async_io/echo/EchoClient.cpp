#include "EchoClient.h"
#include "reactor/Reactor.h"
#include "async/AsyncBuffer.h"
#include <iostream>
#include <cstring>

EchoClient::EchoClient(Reactor& reactor, int stdin_fd, int write_fd, int read_fd)
    : _reactor(reactor), _stdin_fd(stdin_fd), _write_fd(write_fd), _read_fd(read_fd) {}

AsyncIoCoroutine EchoClient::run() {
    std::cout << "[Client] Started" << std::endl;
    
    char write_buffer[256];
    char read_buffer[256];
    
    while (true) {
        // Read from stdin until we get a complete line (ends with \n)
        std::cout << "[Client] Waiting for input..." << std::endl;
        size_t total = 0;
        
        while (total < sizeof(write_buffer) - 1) {
            ssize_t n = co_await AsyncReadBuffer<>{_reactor, _stdin_fd,
                                                    std::span<char>(write_buffer + total, sizeof(write_buffer) - total)};
            
            if (n <= 0) {
                std::cout << "[Client] EOF on stdin" << std::endl;
                _reactor.stop();
                co_return;
            }
            
            total += n;
            
            // Check if the last byte is a newline
            if (write_buffer[total - 1] == '\n') {
                break;  // Complete line received
            }
        }
        
        std::string_view input(write_buffer, total);
        std::cout << "[Client] Read from stdin: " << input;
        
        // Write EXACTLY total bytes to pipe1 (loop until all written)
        size_t written = co_await AsyncWriteBuffer<true>{_reactor, _write_fd,
                                                          std::span<const char>(write_buffer, total)};
        
        if (written < total) {
            std::cout << "[Client] ✗ ERROR: Failed to write all bytes to pipe1!" << std::endl;
            std::cout << "  Expected " << total << " bytes, wrote " << written << " bytes" << std::endl;
            break;
        }
        std::cout << "[Client] Wrote " << written << " bytes to pipe1" << std::endl;
        
        // Read EXACTLY total bytes from pipe2 (loop until all read)
        size_t echoed = co_await AsyncReadBuffer<true>{_reactor, _read_fd,
                                                        std::span<char>(read_buffer, total)};
        
        if (echoed < total) {
            std::cout << "[Client] ✗ ERROR: Failed to read all bytes from pipe2!" << std::endl;
            std::cout << "  Expected " << total << " bytes, got " << echoed << " bytes" << std::endl;
            break;
        }
        
        // Verify echo matches
        if (std::memcmp(write_buffer, read_buffer, total) == 0) {
            std::string_view echo(read_buffer, total);
            std::cout << "[Client] Read from pipe2: " << echo;
            std::cout << "[Client] ✓ Echo verified successfully!" << std::endl;
        } else {
            std::cout << "[Client] ✗ ERROR: Echo mismatch!" << std::endl;
            break;
        }
    }
    
    std::cout << "[Client] Stopping reactor" << std::endl;
    _reactor.stop();
}
