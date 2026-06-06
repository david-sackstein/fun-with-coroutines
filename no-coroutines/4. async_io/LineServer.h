#pragma once

#include "common/async_io/LineHandler.h"
#include "common/async_io/LineTransport.h"
#include "common/reactor/WorkGuard.h"

#include <memory>
#include <string>

namespace no_coroutines {

class LineServer {
public:
    explicit LineServer(const async_io::LineTransport &transport);

    void run(async_io::LineHandler handler);

private:
    async_io::LineTransport _transport;

    std::unique_ptr<WorkGuard> _work_guard;
    async_io::LineHandler _handler;

    char _request_buffer[256]{};
    std::string _response;

    void async_read_message();
    void on_read_complete(size_t bytes_read);

    void async_write_response(size_t size);
    void on_write_complete(size_t expected, size_t actual);

    static void verify_write_complete(size_t expected, size_t actual);
};

}
