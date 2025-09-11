#pragma once

// Timing constants used in demos and tests to simulate async work and
// provide safety margins that prevent the event loop or reactor from
// hanging indefinitely if an operation never completes.

#include <chrono>

namespace testing_delay {

// How long each simulated async task sleeps on its background thread.
inline constexpr auto async_task = std::chrono::seconds{1};
// How long the main thread waits for unmarshalled background tasks to complete.
inline constexpr auto unmarshalled_wait = std::chrono::seconds{5};
// Safety timeout: stops the Reactor if all work is otherwise done.
inline constexpr auto reactor_safety = std::chrono::seconds{10};
// Safety timeout for interactive REPL sessions.
inline constexpr auto repl_safety = std::chrono::seconds{30};

}
