#pragma once

#include "coroutines/common/Task.h"

#include <array>

class Reactor;

namespace coroutines {

Task<std::array<int, 4>> solve_quiz(Reactor &reactor, int write_fd, int read_fd);

[[nodiscard]] std::array<int, 4> run_calculator_batch();

}
