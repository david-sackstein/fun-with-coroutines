#pragma once

// Demo 05 - task composition. Appropriate because the coroutine produces a single
// result and can co_await other Tasks, enabling coroutine chaining without callbacks.
//
// Coroutine return type for a lazy task that produces a single result value.
// Task<T> is also an awaitable, so one Task can co_await another (coroutine chaining).
//
// promise_type hooks
// initial_suspend   suspend_always          - starts suspended; driven by co_await or sync_wait
// final_suspend     FinalAwaiter<promise>   - resumes the stored continuation via symmetric transfer
// return_void       no
// return_value      yes                     - co_return value stores it in promise._result
// yield_value       no

#include "coroutines/return_types/task/FinalAwaiter.h"

#include <coroutine>
#include <exception>
#include <utility>

namespace coroutines {

template<typename T>
struct Task {

    // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() {
            return {};
        }

        FinalAwaiter<promise_type> final_suspend() noexcept {
            return {};
        }

        void return_value(T value) {
            _result = std::move(value);
        }

        void unhandled_exception() {
            std::terminate();
        }

        T _result{};

        // Set by await_suspend when another coroutine does co_await task.
        std::coroutine_handle<> _continuation{};
    };
    // NOLINTEND(readability-convert-member-functions-to-static)

    explicit Task(std::coroutine_handle<promise_type> handle) : _handle(handle) {}

    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;

    Task(Task &&other) noexcept : _handle(std::exchange(other._handle, {})) {}

    Task &operator=(Task &&other) noexcept {
        if (this != &other) {
            // Destroy the current frame before taking ownership of the new one.
            destroy();
            _handle = std::exchange(other._handle, {});
        }
        return *this;
    }

    ~Task() {
        destroy();
    }

    // -- awaitable interface (not present on other return types) --------------
    //
    // These three methods make Task itself usable as the operand of co_await,
    // so one Task coroutine can suspend until another Task completes:
    //
    //   Task<int> outer() { int x = co_await inner(); co_return x + 1; }
    //
    // await_suspend stores the outer coroutine handle as the continuation and
    // returns the inner handle (symmetric transfer) to start the inner Task.

    // NOLINTBEGIN(readability-convert-member-functions-to-static) -- co_await awaitable
    bool await_ready() noexcept {
        return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) noexcept {
        _handle.promise()._continuation = caller;
        return _handle;
    }

    T await_resume() {
        return std::move(_handle.promise()._result);
    }
    // NOLINTEND(readability-convert-member-functions-to-static)

    // -- synchronous entry point -----------------------------------------------

    // Start the coroutine from its initial suspension; called by sync_wait.
    void start() {
        if (_handle && !_handle.done()) {
            _handle.resume();
        }
    }

    // Read the result after the chain has completed; called by sync_wait.
    [[nodiscard]] T result() {
        return std::move(_handle.promise()._result);
    }

private:
    // Destroy the coroutine frame and clear the handle.
    void destroy() {
        if (_handle) {
            _handle.destroy();
            _handle = {};
        }
    }

    std::coroutine_handle<promise_type> _handle;
};

// Drive a top-level Task to completion from synchronous code and return its result.
template<typename T>
T sync_wait(Task<T> task) {
    task.start();
    return task.result();
}

}
