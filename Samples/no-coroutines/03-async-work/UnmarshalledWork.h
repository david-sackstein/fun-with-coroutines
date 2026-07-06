#pragma once

// Callback-based equivalent of the coroutines UnmarshalledWork demo.
// Performs two async steps with nested callbacks; each continuation runs on a background thread.
// Compare with the coroutines version where the same sequence is expressed with co_await.

#include <string>
#include <vector>

namespace no_coroutines {

// Run a callback chain of two unmarshalled async steps and return the thread ID at each step.
[[nodiscard]] std::vector<std::string> collect_unmarshalled_thread_ids();

}
