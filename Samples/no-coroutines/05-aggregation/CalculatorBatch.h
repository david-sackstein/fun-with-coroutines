#pragma once

// Callback-based batch calculator client.
// SolveQuiz (internal) expresses the same question/answer loop as the coroutines
// solve_quiz Task, but using a callback chain instead of co_await expressions.

#include <array>

namespace no_coroutines {

// Set up the pipes, server, and quiz callback chain, run the Reactor, and return the answers.
[[nodiscard]] std::array<int, 4> run_calculator_batch();

}
