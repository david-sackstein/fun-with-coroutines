#pragma once

// Callback-based equivalent of the coroutines MarshalledTask demo.
// Performs three async steps with nested callbacks; each continuation runs on the EventLoop thread.
// Compare with the coroutines version where the same sequence is expressed with co_await.

#include <string>
#include <vector>

class EventLoop;

namespace no_coroutines {

// Run a callback chain of three marshalled async steps and return the thread ID at each step.
[[nodiscard]] std::vector<std::string> collect_marshalled_thread_ids(EventLoop &loop);

}
