#pragma once

#include "common/event_loop/EventLoop.h"

#include <functional>
#include <thread>

namespace no_coroutines {

    using namespace std::chrono_literals;

    class AsyncTaskRunnerMarshalled {
    public:
        explicit AsyncTaskRunnerMarshalled(EventLoop& loop) : loop(loop) {}

        void run_async_operation(const std::function<void()>& continuation) const {
            std::thread t([continuation, &loop = this->loop]() mutable {
                std::this_thread::sleep_for(1s);
                loop.post(continuation);
            });
            t.detach();
        }

    private:
        EventLoop& loop;
    };
}
