#pragma once

#include "common/reactor/Reactor.h"
#include "common/reactor/WorkGuard.h"

#include <memory>

namespace no_coroutines {

class EchoServer {
public:
    EchoServer(Reactor& reactor, int read_fd, int write_fd);
    
    void run();

private:
    Reactor& _reactor;
    int _read_fd;
    int _write_fd;
    
    std::unique_ptr<WorkGuard> _work_guard;
    
    char _buffer[256];
    
    // Async operation helpers
    void async_read_message();
    void on_read_complete(size_t bytes_read);
    
    void async_write_echo(size_t size);
    void on_write_complete(size_t expected, size_t actual);
    
    // Utility methods
    void log_received_message(const char *data, size_t size);
    void verify_write_complete(size_t expected, size_t actual);
};

}
