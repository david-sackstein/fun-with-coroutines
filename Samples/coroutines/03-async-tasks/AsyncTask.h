#pragma once

// Awaitable that runs user-provided work on a background thread and resumes
// the coroutine directly on that same thread (unmarshalled).
//
// On co_await: a detached thread runs Work, then calls h.resume() directly.
// The coroutine continues on the background thread.
// Compare with AsyncTaskMarshalled, which posts the resume to the EventLoop thread.

#include <coroutine>
#include <thread>
#include <utility>

namespace coroutines {

template<typename Work>
struct AsyncTask {
    explicit AsyncTask(Work work) : _work(std::move(work)) {}

    // NOLINTBEGIN(readability-convert-member-functions-to-static) -- co_await awaitable

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> h) {
        std::thread t([this, h] {
            _work();
            h.resume();
        });
        t.detach();
    }

    void await_resume() {}

    // NOLINTEND(readability-convert-member-functions-to-static)

private:
    Work _work;
};

}
