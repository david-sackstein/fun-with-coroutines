#pragma once

// Coroutine that accumulates a total across three marshalled async steps.
// Each co_await suspends, does work on a background thread, and resumes on the EventLoop thread.
// The result is returned via co_return and collected with Task::get().

#include "coroutines/return_types/Task.h"

class EventLoop;

namespace coroutines {

// Perform three async additions and co_return the total.
Task<int> aggregate_totals(EventLoop &loop);

}
