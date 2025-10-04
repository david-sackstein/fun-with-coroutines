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
        // Read from stdin until newline
        std::cout << "[Client] Waiting for input..." << std::endl;
        size_t total = co_await AsyncReadUntil<'\n'>{_reactor, _stdin_fd, write_buffer};
        
        if (total == 0) {
            std::cout << "[Client] EOF on stdin" << std::endl;
            _reactor.stop();
            co_return;
        }
        
        log_input(std::span<const char>(write_buffer, total));
        
        // Write to pipe1
        size_t written = co_await AsyncWriteExact<>{_reactor, _write_fd,
                                                     std::span<const char>(write_buffer, total)};
        
        if (written < total) {
            std::cout << "[Client] ✗ ERROR: Failed to write all bytes to pipe1!" << std::endl;
            std::cout << "  Expected " << total << " bytes, wrote " << written << " bytes" << std::endl;
            break;
        }
        log_write_result(written);
        
        // Read from pipe2
        size_t echoed = co_await AsyncReadExact<>{_reactor, _read_fd,
                                                   std::span<char>(read_buffer, total)};
        
        if (echoed < total) {
            std::cout << "[Client] ✗ ERROR: Failed to read all bytes from pipe2!" << std::endl;
            std::cout << "  Expected " << total << " bytes, got " << echoed << " bytes" << std::endl;
            break;
        }
        
        // Verify echo
        if (!verify_and_log_echo(std::span<const char>(write_buffer, total),
                                 std::span<const char>(read_buffer, echoed))) {
            std::cout << "[Client] ✗ ERROR: Echo mismatch!" << std::endl;
            break;
        }
    }
    
    std::cout << "[Client] Stopping reactor" << std::endl;
    _reactor.stop();
}

void EchoClient::log_input(std::span<const char> data) {
    std::string_view input(data.data(), data.size());
    std::cout << "[Client] Read from stdin: " << input;
}

void EchoClient::log_write_result(size_t bytes_written) {
    std::cout << "[Client] Wrote " << bytes_written << " bytes to pipe1" << std::endl;
}

bool EchoClient::verify_and_log_echo(std::span<const char> sent, std::span<const char> received) {
    if (std::memcmp(sent.data(), received.data(), sent.size()) == 0) {
        std::string_view echo(received.data(), received.size());
        std::cout << "[Client] Read from pipe2: " << echo;
        std::cout << "[Client] ✓ Echo verified successfully!" << std::endl;
        return true;
    }
    return false;
}