#pragma once

#include "common/reactor/Reactor.h"
#include "common/reactor/WorkGuard.h"

#include <memory>
#include <span>

namespace no_coroutines {

class EchoClient {
public:
    EchoClient(Reactor& reactor, int stdin_fd, int write_fd, int read_fd);
    
    void run();

private:
    Reactor& _reactor;
    int _stdin_fd;
    int _write_fd;
    int _read_fd;
    
    std::unique_ptr<WorkGuard> _work_guard;
    
    char _write_buffer[256];
    char _read_buffer[256];
    
    // Async operation helpers
    void async_read_from_stdin();
    void on_stdin_read_complete(size_t bytes_read);
    
    void async_write_to_server(size_t size);
    void on_write_complete(size_t expected, size_t actual);
    
    void async_read_echo(size_t expected);
    void on_echo_complete(size_t expected, size_t actual);
    
    // Utility methods
    void log_input(const char *data, size_t size);
    void verify_write_complete(size_t expected, size_t actual);
    void verify_read_complete(size_t expected, size_t actual);
    void verify_and_log_echo(const char *sent, size_t sent_size, 
                             const char *received, size_t received_size);
};

}
