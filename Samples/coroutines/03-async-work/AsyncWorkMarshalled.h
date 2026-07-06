#pragma once

// Awaitable that runs user-provided work on a background thread and marshals
// the coroutine resume back to the EventLoop thread.
//
// On co_await: a detached thread runs Work, then posts h.resume() to the EventLoop.
// The coroutine therefore continues on the EventLoop thread, not on the background thread.
// Compare with AsyncWork, which resumes directly on the background thread.

#include "common/event_loop/EventLoop.h"

#include <coroutine>
#include <thread>
#include <utility>

namespace coroutines {

// NOLINTBEGIN(readability-convert-member-functions-to-static) -- co_await awaitable
template<typename Work>
struct AsyncWorkMarshalled {
    AsyncWorkMarshalled(EventLoop &loop, Work work) : _loop(loop), _work(std::move(work)) {}

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> h) const {
        std::thread t([this, h] {
            _work();
            _loop.post([h] { h.resume(); });
        });
        t.detach();
    }

    void await_resume() {}

private:
    EventLoop &_loop;
    Work _work;
};
// NOLINTEND(readability-convert-member-functions-to-static)

}
