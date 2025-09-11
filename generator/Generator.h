#pragma once

#include <coroutine>
#include <print>

struct Generator {

    struct promise_type {
        Generator get_return_object() {
            return Generator(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception(){}

        std::suspend_always yield_value(int value) {
            current_value = value;
            return {};
        }

        int current_value;
    };

    Generator(std::coroutine_handle<promise_type> handle) :
        _handle(handle)
    {
    }

    ~Generator()
    {
        if (_handle) {
            _handle.destroy();
        }
    }

    bool next() {
        if (_handle.done()) {
            return false;
        }
        _handle.resume();
        return !_handle.done();
    }

    int get_current_value() {
        auto& promise = _handle.promise();
        return promise.current_value;
    }

    std::coroutine_handle<promise_type> _handle;
};