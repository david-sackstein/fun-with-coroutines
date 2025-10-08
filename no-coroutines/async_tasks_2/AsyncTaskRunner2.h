#pragma once

#include <functional>
#include <thread>

namespace no_coroutines {

    using namespace std::chrono_literals;

    // Helper class to manage async task state without event loop
    class AsyncTaskRunner2 {
    public:
        void run_async_operation(std::function<void()> continuation) {
            // Spawn a thread that sleeps for 1 second, then calls the continuation directly on this thread
            std::thread t([continuation] {
                std::this_thread::sleep_for(1s);
                // Call continuation directly on this thread, not posting to event loop
                continuation();
            });
            t.detach();
        }
    };
}
