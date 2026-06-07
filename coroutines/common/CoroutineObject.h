#pragma once

#include <coroutine>
#include <exception>

namespace coroutines {

// promise_type hooks
// initial_suspend   suspend_never
// final_suspend     suspend_never
// return_void       yes
// return_value      —
// yield_value       —
struct CoroutineObject {
    struct promise_type {
        CoroutineObject get_return_object() {
            return CoroutineObject{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }

        void return_void() {}

        void unhandled_exception() { std::terminate(); }
        // NOLINTEND(readability-convert-member-functions-to-static)
    };

    explicit CoroutineObject(std::coroutine_handle<promise_type> handle) : _handle(handle) {}

    CoroutineObject(const CoroutineObject &) = delete;
    CoroutineObject &operator=(const CoroutineObject &) = delete;

    CoroutineObject(CoroutineObject &&) = delete;
    CoroutineObject &operator=(CoroutineObject &&) = delete;

private:
    std::coroutine_handle<promise_type> _handle;
};

}
