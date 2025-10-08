#include "EchoClient.h"

#include <print>
#include <stdexcept>
#include <unistd.h>
#include <cerrno>
#include <cstring>

namespace no_coroutines {

EchoClient::EchoClient(Reactor &reactor, int stdin_fd, int write_fd, int read_fd)
    : _reactor(reactor), _stdin_fd(stdin_fd), _write_fd(write_fd), _read_fd(read_fd) {}

void EchoClient::run() {
    _work_guard = std::make_unique<WorkGuard>(_reactor);
    std::print("[Client] Started\n");
    async_read_from_stdin();
}

// ============================================================================
// Async read from stdin until newline
// ============================================================================

void EchoClient::async_read_from_stdin() {
    std::print("[Client] Waiting for input...\n");
    
    auto self = std::make_shared<size_t>(0);  // offset tracker
    
    auto read_handler = [this, self](int fd) mutable {
        ssize_t n = ::read(fd, _write_buffer + *self, sizeof(_write_buffer) - *self);
        
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
            // Need to retry - re-post
            _reactor.post(_stdin_fd, Reactor::FdMode::Read, [this, self](int fd) mutable {
                this->async_read_from_stdin();
            });
            return;
        }
        
        if (n <= 0) {
            // EOF or error
            _reactor.remove(_stdin_fd, Reactor::FdMode::Read);
            on_stdin_read_complete(0);
            return;
        }
        
        // Success: n > 0
        *self += n;
        
        // Check if we found newline or need more data
        bool found_newline = (*self > 0 && _write_buffer[*self - 1] == '\n');
        bool buffer_full = (*self >= sizeof(_write_buffer));
        
        if (!found_newline && !buffer_full) {
            // Need more data - re-post
            _reactor.post(_stdin_fd, Reactor::FdMode::Read, [this, self](int fd) mutable {
                auto read_handler_inner = [this, self](int fd) mutable {
                    ssize_t n2 = ::read(fd, _write_buffer + *self, sizeof(_write_buffer) - *self);
                    
                    if (n2 < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
                        // Retry
                        _reactor.post(_stdin_fd, Reactor::FdMode::Read, [this, self](int) { async_read_from_stdin(); });
                        return;
                    }
                    
                    if (n2 <= 0) {
                        _reactor.remove(_stdin_fd, Reactor::FdMode::Read);
                        on_stdin_read_complete(*self);
                        return;
                    }
                    
                    *self += n2;
                    bool found = (*self > 0 && _write_buffer[*self - 1] == '\n');
                    
                    if (found || *self >= sizeof(_write_buffer)) {
                        _reactor.remove(_stdin_fd, Reactor::FdMode::Read);
                        on_stdin_read_complete(*self);
                    } else {
                        _reactor.post(_stdin_fd, Reactor::FdMode::Read, [this, self](int) { async_read_from_stdin(); });
                    }
                };
                _reactor.post(_stdin_fd, Reactor::FdMode::Read, read_handler_inner);
            });
            return;
        }
        
        // Done reading
        _reactor.remove(_stdin_fd, Reactor::FdMode::Read);
        on_stdin_read_complete(*self);
    };
    
    _reactor.post(_stdin_fd, Reactor::FdMode::Read, read_handler);
}

void EchoClient::on_stdin_read_complete(size_t bytes_read) {
    if (bytes_read == 0) {
        std::print("[Client] EOF on stdin\n");
        close(_write_fd);
        std::print("[Client] Finished\n");
        _work_guard.reset();  // Release work guard to stop reactor
        return;
    }
    
    log_input(_write_buffer, bytes_read);
    async_write_to_server(bytes_read);
}

// ============================================================================
// Async write exact amount to server
// ============================================================================

void EchoClient::async_write_to_server(size_t size) {
    auto offset = std::make_shared<size_t>(0);
    auto total = std::make_shared<size_t>(size);
    
    std::function<void(int)> write_handler;
    write_handler = [this, offset, total, write_handler](int fd) mutable {
        ssize_t n = ::write(fd, _write_buffer + *offset, *total - *offset);
        
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

void EchoClient::on_write_complete(size_t expected, size_t actual) {
    verify_write_complete(expected, actual);
    async_read_echo(expected);
}

// ============================================================================
// Async read exact amount from server
// ============================================================================

void EchoClient::async_read_echo(size_t expected) {
    auto offset = std::make_shared<size_t>(0);
    auto total = std::make_shared<size_t>(expected);
    
    std::function<void(int)> read_handler;
    read_handler = [this, offset, total, read_handler](int fd) mutable {
        ssize_t n = ::read(fd, _read_buffer + *offset, *total - *offset);
        
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
            // Retry
            _reactor.post(_read_fd, Reactor::FdMode::Read, read_handler);
            return;
        }
        
        if (n <= 0) {
            // EOF or error
            _reactor.remove(_read_fd, Reactor::FdMode::Read);
            on_echo_complete(*total, *offset);
            return;
        }
        
        *offset += n;
        
        if (*offset < *total) {
            // Need to read more
            _reactor.post(_read_fd, Reactor::FdMode::Read, read_handler);
        } else {
            // Done
            _reactor.remove(_read_fd, Reactor::FdMode::Read);
            on_echo_complete(*total, *offset);
        }
    };
    
    _reactor.post(_read_fd, Reactor::FdMode::Read, read_handler);
}

void EchoClient::on_echo_complete(size_t expected, size_t actual) {
    verify_read_complete(expected, actual);
    verify_and_log_echo(_write_buffer, expected, _read_buffer, actual);
    
    // Continue the loop - read next input
    async_read_from_stdin();
}

// ============================================================================
// Utility methods
// ============================================================================

void EchoClient::log_input(const char *data, size_t size) {
    std::print("[Client] Read from stdin: {}", std::string_view(data, size));
}

void EchoClient::verify_write_complete(size_t expected, size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format(
            "[Client] Failed to write all bytes to pipe_client_to_server! Expected {} bytes, wrote {} bytes", expected,
            actual));
    }
    std::print("[Client] Wrote {} bytes to pipe_client_to_server\n", actual);
}

void EchoClient::verify_read_complete(size_t expected, size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(
            std::format("[Client] Failed to read all bytes from pipe_server_to_client! Expected {} bytes, got {} bytes",
                        expected, actual));
    }
}

void EchoClient::verify_and_log_echo(const char *sent, size_t sent_size,
                                     const char *received, size_t received_size) {
    if (std::memcmp(sent, received, sent_size) != 0) {
        throw std::runtime_error("[Client] Echo mismatch!");
    }
    std::print("[Client] Read from pipe_server_to_client: {}", std::string_view(received, received_size));
    std::print("[Client] ✓ Echo verified successfully!\n");
}

}
