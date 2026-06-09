#pragma once

#include "common/event_loop/EventLoop.h"
#include "common/testing/Delays.h"

#include <coroutine>
#include <thread>

namespace coroutines {
    struct AsyncTaskMarshalled {
        EventLoop &loop;

        // NOLINTBEGIN(readability-convert-member-functions-to-static) -- co_await awaitable
        bool await_ready() { return false; }

        void await_suspend(std::coroutine_handle<> h) const {
            std::thread t([=, this] {
                std::this_thread::sleep_for(testing_delay::async_task);
                loop.post([=] {
                    h.resume();
                });
            });
            t.detach();
        }

        void await_resume() {}
        // NOLINTEND(readability-convert-member-functions-to-static)
    };
}
