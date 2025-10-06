#pragma once

#include "common/event_loop/EventLoop.h"

#include <functional>
#include <thread>

namespace no_coroutines {

    using namespace std::chrono_literals;

    // Helper class to manage async task state without coroutines
    class AsyncTaskRunner {
    public:
        AsyncTaskRunner(EventLoop& loop) : loop(loop), work_guard(loop) {}

        void run_async_operation(std::function<void()> continuation) {
            // Spawn a thread that sleeps for 1 second, then posts the continuation to the event loop
            std::thread t([continuation, &loop = this->loop] {
                std::this_thread::sleep_for(1s);
                loop.post(continuation);
            });
            t.detach();
        }

    private:
        EventLoop& loop;
        EventLoop::Work work_guard;
    };
}
