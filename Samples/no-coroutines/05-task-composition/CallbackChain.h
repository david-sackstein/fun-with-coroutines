#pragma once

// Callback-based equivalent of the coroutines TaskChain.
// Three nested callbacks accumulate the same partial sums, but the
// result is delivered via on_complete instead of co_return.
// Compare with the coroutines version where the sequence is a single function body.

#include <functional>
#include <memory>

class EventLoop;

namespace no_coroutines {

// Start a nested callback chain that adds 10+20+30 asynchronously and calls on_complete with the sum.
// keepalive must remain valid until on_complete fires; reset it inside on_complete to let the loop stop.
void chain_callbacks(
    EventLoop &loop,
    std::shared_ptr<EventLoop::Work> *keepalive,
    const std::function<void(int)> &on_complete);

// Wire up chain_callbacks with a keepalive, run the loop, and return the result.
[[nodiscard]] int run_callback_chain(EventLoop &loop);

}
