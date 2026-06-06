#pragma once

#include "common/async_io/LineTransport.h"
#include "common/reactor/Reactor.h"

#include <functional>

namespace no_coroutines {

class LineRpc {
public:
    explicit LineRpc(const async_io::LineTransport &transport);

    void round_trip(async_io::LineRpcCall &call, std::function<void()> on_complete);

private:
    async_io::LineTransport _transport;

    async_io::LineRpcCall *_call{};
    std::function<void()> _on_complete;

    void async_write_request() const;
    void on_write_complete(size_t expected, size_t actual) const;

    void async_read_response() const;
    void on_read_complete(size_t bytes_read) const;

    static void verify_write_complete(size_t expected, size_t actual);
};

}
