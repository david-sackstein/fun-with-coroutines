#pragma once

// solve_quiz — coroutine that sends arithmetic questions to a CalcServer sequentially,
// reads each response, and co_returns the accumulated answers as a vector.
// Verification of the answers is the caller's responsibility.

#include "coroutines/return_types/task/Task.h"

#include <span>
#include <string_view>
#include <vector>

namespace coroutines {

class Reactor;

[[nodiscard]] Task<std::vector<int>> solve_quiz(
    Reactor &reactor,
    int write_fd, int read_fd,
    std::span<const std::string_view> questions);

}
