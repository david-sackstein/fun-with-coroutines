#pragma once

#include <thread>
#include <coroutine>

using namespace std::chrono_literals;

namespace coroutines {
    struct AsyncTask {
        bool await_ready() { return false; }

        void await_suspend(std::coroutine_handle<> h) {
            std::thread t([=] {
                std::this_thread::sleep_for(1s);
                // Resume directly on this thread, not posting to event loop
                h.resume();
            });
            t.detach();
        }

        void await_resume() {}
    };
}
