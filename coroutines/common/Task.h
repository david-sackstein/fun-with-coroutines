#pragma once

#include <coroutine>
#include <exception>
#include <stdexcept>
#include <utility>

namespace coroutines {

template<typename T>
struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void return_value(T value) { _result = std::move(value); }

        void unhandled_exception() { std::terminate(); }
        // NOLINTEND(readability-convert-member-functions-to-static)

        T _result{};
    };

    explicit Task(std::coroutine_handle<promise_type> handle) : _handle(handle) {}

    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;

    Task(Task &&other) noexcept : _handle(std::exchange(other._handle, {})) {}

    Task &operator=(Task &&other) noexcept {
        if (this != &other) {
            destroy();
            _handle = std::exchange(other._handle, {});
        }
        return *this;
    }

    ~Task() { destroy(); }

    void start() {
        if (_handle && !_handle.done()) {
            _handle.resume();
        }
    }

    [[nodiscard]] T get() {
        if (!_handle) {
            throw std::logic_error("Task::get() on empty task");
        }

        start();

        if (!_handle.done()) {
            throw std::runtime_error("Task not complete — drive the event loop or reactor first");
        }

        return std::move(_handle.promise()._result);
    }

private:
    void destroy() {
        if (_handle) {
            _handle.destroy();
            _handle = {};
        }
    }

    std::coroutine_handle<promise_type> _handle;
};

}
