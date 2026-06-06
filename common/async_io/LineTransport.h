#pragma once

#include "common/reactor/Reactor.h"

#include <span>
#include <string_view>

namespace async_io {

struct LineTransport {
    Reactor &reactor;
    int read_fd;
    int write_fd;
};

struct LineRpcCall {
    std::string_view request;
    std::span<char> response;
    size_t response_size = 0;
};

}
