#pragma once

// Demo that shows a coroutine resuming on the EventLoop thread after each async step.
// Each co_await AsyncTaskMarshalled suspends, does work on a background thread,
// and then resumes the coroutine back on the EventLoop thread.
// The collected thread IDs show that every step runs on the same loop thread.

#include <string>
#include <vector>

class EventLoop;

namespace coroutines {

// Run a coroutine that performs three marshalled async steps and return the thread ID at each step.
[[nodiscard]] std::vector<std::string> collect_marshalled_thread_ids(EventLoop &loop);

}
