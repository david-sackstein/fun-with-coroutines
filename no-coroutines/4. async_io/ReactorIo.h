#pragma once

#include "common/reactor/Reactor.h"

#include <cstddef>
#include <functional>

namespace no_coroutines {

using IoComplete = std::function<void(size_t bytes_done)>;
using ReadStopWhen = std::function<bool(const char *buffer, size_t offset, size_t limit)>;

[[nodiscard]] ReadStopWhen stop_at_newline_or_full();
[[nodiscard]] ReadStopWhen stop_at_exact_bytes();

void post_read(
    Reactor &reactor,
    int fd,
    char *buffer,
    size_t limit,
    ReadStopWhen stop_when,
    IoComplete on_complete);

void post_write(
    Reactor &reactor,
    int fd,
    const char *buffer,
    size_t total,
    IoComplete on_complete);

}
