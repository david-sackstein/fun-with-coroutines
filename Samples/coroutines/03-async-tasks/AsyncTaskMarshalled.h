#pragma once

// Awaitable that runs user-provided work on a background thread and marshals
// the coroutine resume back to the EventLoop thread.
//
// On co_await: a detached thread runs Work, then posts h.resume() to the EventLoop.
// The coroutine therefore continues on the EventLoop thread, not on the background thread.
// Compare with AsyncTask, which resumes directly on the background thread.

#include "common/event_loop/EventLoop.h"

#include <coroutine>
#include <thread>
#include <utility>

namespace coroutines {

template<typename Work>
struct AsyncTaskMarshalled {
    AsyncTaskMarshalled(EventLoop &loop, Work work) : _loop(loop), _work(std::move(work)) {}

    // NOLINTBEGIN(readability-convert-member-functions-to-static) -- co_await awaitable

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> h) const {
        std::thread t([this, h] {
            _work();
            _loop.post([h] { h.resume(); });
        });
        t.detach();
    }

    void await_resume() {}

    // NOLINTEND(readability-convert-member-functions-to-static)

private:
    EventLoop &_loop;
    Work _work;
};

}
