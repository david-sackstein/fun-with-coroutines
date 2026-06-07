#pragma once

#include "common/generator/Iterator.h"

#include <coroutine>
#include <exception>
#include <utility>

namespace coroutines {

struct Generator {
    struct promise_type {
        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void return_void() {}

        void unhandled_exception() { std::terminate(); }
        // NOLINTEND(readability-convert-member-functions-to-static)

        std::suspend_always yield_value(const int value) {
            _current_value = value;
            return {};
        }

        int _current_value{};
    };

    explicit Generator(std::coroutine_handle<promise_type> handle) : _handle(handle) {}

    Generator(const Generator &) = delete;
    Generator &operator=(const Generator &) = delete;

    Generator(Generator &&other) noexcept : _handle(std::exchange(other._handle, {})) {}

    Generator &operator=(Generator &&other) noexcept {
        if (this != &other) {
            destroy();
            _handle = std::exchange(other._handle, {});
        }
        return *this;
    }

    ~Generator() { destroy(); }

    // NOLINTNEXTLINE(readability-make-member-function-const) -- resume() mutates coroutine state
    bool next() {
        if (_handle.done()) {
            return false;
        }
        _handle.resume();
        return !_handle.done();
    }

    [[nodiscard]] int get_current_value() const { return _handle.promise()._current_value; }

private:
    void destroy() {
        if (_handle) {
            _handle.destroy();
            _handle = {};
        }
    }

    std::coroutine_handle<promise_type> _handle;
};

inline GeneratorIterator<Generator> begin(Generator &generator) {
    return GeneratorIterator{generator};
}

inline GeneratorIterator<Generator> end(const Generator &) {
    return {};
}

}
