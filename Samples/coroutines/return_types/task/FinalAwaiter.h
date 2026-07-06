#pragma once

// Coroutine awaiter returned by Task<T>::promise_type::final_suspend.
//
// Other return types return suspend_always from final_suspend, which simply parks
// the frame. FinalAwaiter instead resumes whatever coroutine is co_await-ing this
// Task, using symmetric transfer: await_suspend returns the continuation handle
// rather than calling resume(), so the compiler emits a tail-call and the stack
// does not grow with each level of chaining.

#include <coroutine>

namespace coroutines {

// NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine awaiter
template<typename Promise>
struct FinalAwaiter {
    bool await_ready() noexcept {
        return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise> h) noexcept {
        if (const auto c = h.promise()._continuation) {
            return c;
        }
        return std::noop_coroutine();
    }

    void await_resume() noexcept {}
};
// NOLINTEND(readability-convert-member-functions-to-static)

}
