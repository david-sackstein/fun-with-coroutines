#pragma once

#include <coroutine>
#include <exception>

namespace coroutines {

// promise_type hooks
// initial_suspend   suspend_never
// final_suspend     suspend_always
// return_void       yes
// return_value      no
// yield_value       no
struct FinalSuspendCoroutine {
    struct promise_type {
        FinalSuspendCoroutine get_return_object() {
            return FinalSuspendCoroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void return_void() {}

        void unhandled_exception() { std::terminate(); }
        // NOLINTEND(readability-convert-member-functions-to-static)
    };

    explicit FinalSuspendCoroutine(std::coroutine_handle<promise_type> handle) : _handle(handle) {}

    FinalSuspendCoroutine(const FinalSuspendCoroutine &) = delete;
    FinalSuspendCoroutine &operator=(const FinalSuspendCoroutine &) = delete;

    FinalSuspendCoroutine(FinalSuspendCoroutine &&) = delete;
    FinalSuspendCoroutine &operator=(FinalSuspendCoroutine &&) = delete;

    ~FinalSuspendCoroutine() {
        if (_handle) {
            _handle.destroy();
        }
    }

private:
    std::coroutine_handle<promise_type> _handle;
};

}
