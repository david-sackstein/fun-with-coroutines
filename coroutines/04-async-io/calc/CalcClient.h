#pragma once

#include "coroutines/return_types/FinalSuspendCoroutine.h"
#include "common/reactor/Reactor.h"

namespace coroutines {

class CalcClient {
public:
    CalcClient(Reactor& reactor, int stdin_fd, int write_fd, int read_fd);
    
    [[nodiscard]] FinalSuspendCoroutine run() const;

private:
    Reactor& _reactor;
    int _stdin_fd;
    int _write_fd;
    int _read_fd;
    
    static void log_input(const char *data, size_t size);
    static void verify_write_complete(size_t expected, size_t actual);
    static void verify_read_complete(size_t expected, size_t actual);
    static void verify_and_log_response(std::string_view received, std::string_view expected);
};

}