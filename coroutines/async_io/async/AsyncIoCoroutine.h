#pragma once

#include <coroutine>
#include <exception>

namespace coroutines {
    struct AsyncIoCoroutine {
        struct promise_type {
            AsyncIoCoroutine get_return_object() {
                return AsyncIoCoroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            std::suspend_never initial_suspend() { return {}; }

            std::suspend_always final_suspend() noexcept { return {}; } // Let me destroy in the dtor
            void return_void() {}

            void unhandled_exception() { std::terminate(); }
        };

        explicit AsyncIoCoroutine(std::coroutine_handle<promise_type> h) : handle(h) {}

        AsyncIoCoroutine(const AsyncIoCoroutine &) = delete;

        AsyncIoCoroutine(AsyncIoCoroutine &&) = delete;

        ~AsyncIoCoroutine() {
            handle.destroy();
        }

    private:
        std::coroutine_handle<promise_type> handle;
    };
}

