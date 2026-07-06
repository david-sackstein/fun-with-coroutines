#pragma once

// Three-stage coroutine chain: each Task co_awaits the previous one.
// Demonstrates that Task<T> is itself an awaitable - coroutine chaining
// replaces .then() callback chains or std::future continuations.

#include "coroutines/return_types/task/Task.h"

namespace coroutines {

// First stage: produces the seed value.
Task<int> inner();

// Second stage: co_awaits inner() and transforms its result.
Task<int> middle();

// Third stage: co_awaits middle() and produces the final value.
Task<int> outer();

// Drive the full chain synchronously and return the result.
[[nodiscard]] int run_task_chain();

}
