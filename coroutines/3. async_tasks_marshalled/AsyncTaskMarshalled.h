#pragma once

#include "common/event_loop/EventLoop.h"

#include <thread>

using namespace std::chrono_literals;

namespace coroutines {
    struct AsyncTaskMarshalled {
        EventLoop &loop;

        bool await_ready() { return false; }

        void await_suspend(std::coroutine_handle<> h) {
            std::thread t([=, this] {
                std::this_thread::sleep_for(1s);
                loop.post([=] {
                    h.resume();
                });
            });
            t.detach();
        }

        void await_resume() {}
    };
}