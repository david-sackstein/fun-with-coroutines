#pragma once

// Coroutine-based batch calculator client.
// solve_quiz sends four arithmetic questions to a CalcServer over a pipe,
// reads and verifies each answer, and co_returns the four results.
// run_calculator_batch wires the server and client together and runs the Reactor.

#include "coroutines/return_types/Task.h"

#include <array>

class Reactor;

namespace coroutines {

// Coroutine that sends each question, reads the answer, verifies it, and co_returns all four results.
Task<std::array<int, 4>> solve_quiz(Reactor &reactor, int write_fd, int read_fd);

// Set up the pipes, server, and quiz coroutine, run the Reactor, and return the answers.
[[nodiscard]] std::array<int, 4> run_calculator_batch();

}
