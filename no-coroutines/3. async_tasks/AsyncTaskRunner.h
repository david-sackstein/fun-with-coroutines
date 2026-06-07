#pragma once

#include "common/testing/Delays.h"

#include <functional>
#include <thread>

namespace no_coroutines {

    // Helper class to manage async task state without event loop
    class AsyncTaskRunner2 {
    public:
        // NOLINTBEGIN(readability-convert-member-functions-to-static)
        void run_async_operation(const std::function<void()>& continuation) {
            std::thread t([continuation] {
                std::this_thread::sleep_for(testing_delay::async_task);
                // Call continuation directly on this thread, not posting to event loop
                continuation();
            });
            t.detach();
        }
        // NOLINTEND(readability-convert-member-functions-to-static)
    };
}
