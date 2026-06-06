#pragma once

#include <coroutine>
#include <exception>
#include <utility>

namespace coroutines {

struct AsyncIoCoroutineAwaiter;

struct AsyncIoCoroutine {
    struct promise_type {
        AsyncIoCoroutine get_return_object() {
            return AsyncIoCoroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
        std::suspend_never initial_suspend() { return {}; }

        auto final_suspend() noexcept {
            struct FinalAwaiter {
                std::coroutine_handle<> continuation;

                bool await_ready() noexcept { return false; }

                std::coroutine_handle<> await_suspend(std::coroutine_handle<> /*self*/) noexcept {
                    return continuation ? continuation : std::noop_coroutine();
                }

                void await_resume() noexcept {}
            };
            return FinalAwaiter{continuation};
        }

        void return_void() {}

        void unhandled_exception() { std::terminate(); }
        // NOLINTEND(readability-convert-member-functions-to-static)

        std::coroutine_handle<> continuation;
    };

    explicit AsyncIoCoroutine(std::coroutine_handle<promise_type> h) : _handle(h) {}

    AsyncIoCoroutine(const AsyncIoCoroutine &) = delete;

    AsyncIoCoroutine(AsyncIoCoroutine &&other) noexcept : _handle(std::exchange(other._handle, {})) {}

    ~AsyncIoCoroutine() {
        if (_handle) {
            _handle.destroy();
        }
    }

    friend AsyncIoCoroutineAwaiter operator co_await(AsyncIoCoroutine &&coroutine);

private:
    std::coroutine_handle<promise_type> _handle;
};

struct AsyncIoCoroutineAwaiter {
    std::coroutine_handle<AsyncIoCoroutine::promise_type> handle;

    [[nodiscard]] bool await_ready() const { return !handle || handle.done(); }

    void await_suspend(std::coroutine_handle<> parent) {
        handle.promise().continuation = parent;
        handle.resume();
    }

    void await_resume() const {}

    ~AsyncIoCoroutineAwaiter() {
        if (handle) {
            handle.destroy();
        }
    }
};

inline AsyncIoCoroutineAwaiter operator co_await(AsyncIoCoroutine &&coroutine) {
    return AsyncIoCoroutineAwaiter{std::exchange(coroutine._handle, {})};
}

}
