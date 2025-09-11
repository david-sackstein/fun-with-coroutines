#pragma once

#include <coroutine>
#include <exception>
#include <stdexcept>
#include <utility>

namespace coroutines {

// Coroutine return type for a coroutine that produces a single result value.
//
// The coroutine starts suspended. The caller drives it with start() (or via the
// event loop / reactor) and retrieves the result with get() once done.
// get() calls start() internally, so a synchronous coroutine can be driven
// entirely by get().
//
// Used in the aggregation demo: each calculator request is a Task<int> whose
// result is collected after the event loop completes all async work.
//
// promise_type hooks
// initial_suspend   suspend_always  — starts suspended; caller drives the first step
// final_suspend     suspend_always  — keeps the frame alive so get() can read the result
// return_void       no
// return_value      yes             — co_return value stores it in promise._result
// yield_value       no
template<typename T>
struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
        std::suspend_always initial_suspend() {
            return {};
        }
        std::suspend_always final_suspend() noexcept {
            return {};
        }

        // Store the co_return value so get() can retrieve it after the coroutine finishes.
        void return_value(T value) {
            _result = std::move(value);
        }

        void unhandled_exception() {
            std::terminate();
        }
        // NOLINTEND(readability-convert-member-functions-to-static)

        // The value produced by co_return, read by get() after the coroutine finishes.
        T _result{};
    };

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

    // Resume the coroutine from its initial suspension point.
    void start() {
        if (_handle && !_handle.done()) {
            _handle.resume();
        }
    }

    // Start the coroutine (if not already started) and return its result.
    // Throws if the coroutine is not yet complete after start() returns.
    [[nodiscard]] T get() {
        if (!_handle) {
            throw std::logic_error("Task::get() on empty task");
        }

        start();

        if (!_handle.done()) {
            // The coroutine suspended mid-way; it needs an event loop or reactor to continue.
            throw std::runtime_error("Task not complete — drive the event loop or reactor first");
        }

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

}
