#pragma once

#include <coroutine>
#include <exception>

namespace coroutines {

// promise_type hooks
// initial_suspend   suspend_never
// final_suspend     suspend_always
// return_void       yes
// return_value      —
// yield_value       —
struct AsyncIoCoroutine {
    struct promise_type {
        AsyncIoCoroutine get_return_object() {
            return AsyncIoCoroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void return_void() {}

        void unhandled_exception() { std::terminate(); }
        // NOLINTEND(readability-convert-member-functions-to-static)
    };

    explicit AsyncIoCoroutine(std::coroutine_handle<promise_type> handle) : _handle(handle) {}

    AsyncIoCoroutine(const AsyncIoCoroutine &) = delete;
    AsyncIoCoroutine &operator=(const AsyncIoCoroutine &) = delete;

    AsyncIoCoroutine(AsyncIoCoroutine &&) = delete;
    AsyncIoCoroutine &operator=(AsyncIoCoroutine &&) = delete;

    ~AsyncIoCoroutine() {
        if (_handle) {
            _handle.destroy();
        }
    }

private:
    std::coroutine_handle<promise_type> _handle;
};

}
