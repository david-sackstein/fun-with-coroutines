#pragma once

#include <coroutine>
#include <exception>

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

    explicit CoroutineObject(std::coroutine_handle<promise_type> h) : handle(h) {}

    CoroutineObject(const CoroutineObject&) = delete;
    CoroutineObject(CoroutineObject&&) = delete;

private:
    std::coroutine_handle<promise_type> handle;
};