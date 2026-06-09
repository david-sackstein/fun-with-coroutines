#pragma once

#include "common/testing/Delays.h"

#include <coroutine>
#include <thread>

namespace coroutines {
    struct AsyncTask {
        // NOLINTBEGIN(readability-convert-member-functions-to-static) -- co_await awaitable
        bool await_ready() { return false; }

        void await_suspend(std::coroutine_handle<> h) {
            std::thread t([=] {
                std::this_thread::sleep_for(testing_delay::async_task);
                // Resume directly on this thread, not posting to event loop
                h.resume();
            });
            t.detach();
        }

        void await_resume() {}
        // NOLINTEND(readability-convert-member-functions-to-static)
    };
}
