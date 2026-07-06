#pragma once

// Coroutines equivalent of the no-coroutines CalculatorBatch demo.
// solve_quiz is a Task<vector<int>> that sends questions sequentially and collects the answers.
// Compare with the no-coroutines version where the same loop is a nested callback state machine.

#include <vector>

namespace coroutines {

// Wire up the server and solve_quiz Task, run the Reactor, and return the collected answers.
[[nodiscard]] std::vector<int> run_calculator_batch();

}
