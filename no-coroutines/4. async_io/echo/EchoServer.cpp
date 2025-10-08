#include "EchoServer.h"

#include <print>
#include <stdexcept>
#include <unistd.h>
#include <cerrno>

namespace no_coroutines {

EchoServer::EchoServer(Reactor &reactor, int read_fd, int write_fd)
    : _reactor(reactor), _read_fd(read_fd), _write_fd(write_fd) {}

void EchoServer::run() {
    _work_guard = std::make_unique<WorkGuard>(_reactor);
    std::print("[Server] Started\n");
    async_read_message();
}

// ============================================================================
// Async read from client until newline
// ============================================================================

void EchoServer::async_read_message() {
    auto offset = std::make_shared<size_t>(0);
    
    std::function<void(int)> read_handler;
    read_handler = [this, offset, read_handler](int fd) mutable {
        ssize_t n = ::read(fd, _buffer + *offset, sizeof(_buffer) - *offset);
        
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
            // Retry
            _reactor.post(_read_fd, Reactor::FdMode::Read, read_handler);
            return;
        }
        
        if (n <= 0) {
            // EOF or error
            _reactor.remove(_read_fd, Reactor::FdMode::Read);
            on_read_complete(0);
            return;
        }
        
        *offset += n;
        
        // Check if we found newline or buffer is full
        bool found_newline = (*offset > 0 && _buffer[*offset - 1] == '\n');
        bool buffer_full = (*offset >= sizeof(_buffer));
        
        if (!found_newline && !buffer_full) {
            // Need more data
            _reactor.post(_read_fd, Reactor::FdMode::Read, read_handler);
        } else {
            // Done
            _reactor.remove(_read_fd, Reactor::FdMode::Read);
            on_read_complete(*offset);
        }
    };
    
    _reactor.post(_read_fd, Reactor::FdMode::Read, read_handler);
}

void EchoServer::on_read_complete(size_t bytes_read) {
    if (bytes_read == 0) {
        std::print("[Server] EOF on pipe_client_to_server\n");
        std::print("[Server] Finished\n");
        _work_guard.reset();  // Release work guard
        return;
    }
    
    log_received_message(_buffer, bytes_read);
    async_write_echo(bytes_read);
}

// ============================================================================
// Async write exact amount back to client
// ============================================================================

void EchoServer::async_write_echo(size_t size) {
    auto offset = std::make_shared<size_t>(0);
    auto total = std::make_shared<size_t>(size);
    
    std::function<void(int)> write_handler;
    write_handler = [this, offset, total, write_handler](int fd) mutable {
        ssize_t n = ::write(fd, _buffer + *offset, *total - *offset);
        
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
            // Retry
            _reactor.post(_write_fd, Reactor::FdMode::Write, write_handler);
            return;
        }
        
        if (n <= 0) {
            // Error
            _reactor.remove(_write_fd, Reactor::FdMode::Write);
            on_write_complete(*total, *offset);
            return;
        }
        
        *offset += n;
        
        if (*offset < *total) {
            // Need to write more
            _reactor.post(_write_fd, Reactor::FdMode::Write, write_handler);
        } else {
            // Done
            _reactor.remove(_write_fd, Reactor::FdMode::Write);
            on_write_complete(*total, *offset);
        }
    };
    
    _reactor.post(_write_fd, Reactor::FdMode::Write, write_handler);
}

void EchoServer::on_write_complete(size_t expected, size_t actual) {
    verify_write_complete(expected, actual);
    
    // Continue the loop - read next message
    async_read_message();
}

// ============================================================================
// Utility methods
// ============================================================================

void EchoServer::log_received_message(const char *data, size_t size) {
    std::print("[Server] Received: {}", std::string_view(data, size));
}

void EchoServer::verify_write_complete(size_t expected, size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format(
            "[Server] Failed to write all bytes to pipe_server_to_client! Expected {} bytes, wrote {} bytes", expected,
            actual));
    }
    std::print("[Server] Echoed {} bytes to pipe_server_to_client\n", actual);
}

}
