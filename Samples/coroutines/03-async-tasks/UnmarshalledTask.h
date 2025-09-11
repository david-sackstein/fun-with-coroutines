#pragma once

// Demo that shows a coroutine resuming on different background threads after each async step.
// Each co_await AsyncTask suspends and resumes directly on the thread that performed the work.
// The collected thread IDs show that each step runs on a different thread.

#include <string>
#include <vector>

namespace coroutines {

// Run a coroutine that performs two unmarshalled async steps and return the thread ID at each step.
[[nodiscard]] std::vector<std::string> collect_unmarshalled_thread_ids();

}
