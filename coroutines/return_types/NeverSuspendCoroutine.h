#pragma once

#include <coroutine>
#include <exception>

namespace coroutines {

// promise_type hooks
// initial_suspend   suspend_never
// final_suspend     suspend_never
// return_void       yes
// return_value      no
// yield_value       no
struct NeverSuspendCoroutine {
    struct promise_type {
        NeverSuspendCoroutine get_return_object() {
            return NeverSuspendCoroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }

        void return_void() {}

        void unhandled_exception() { std::terminate(); }
        // NOLINTEND(readability-convert-member-functions-to-static)
    };

    explicit NeverSuspendCoroutine(std::coroutine_handle<promise_type> handle) : _handle(handle) {}

    NeverSuspendCoroutine(const NeverSuspendCoroutine &) = delete;
    NeverSuspendCoroutine &operator=(const NeverSuspendCoroutine &) = delete;

    NeverSuspendCoroutine(NeverSuspendCoroutine &&) = delete;
    NeverSuspendCoroutine &operator=(NeverSuspendCoroutine &&) = delete;

private:
    std::coroutine_handle<promise_type> _handle;
};

}
