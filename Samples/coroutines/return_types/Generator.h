#pragma once

// Used in Demo 01 - generator.
// Appropriate because the coroutine produces a lazy sequence:
// values are pulled one at a time via co_yield and the caller drives all iteration.

#include "common/generator/Iterator.h"

#include <coroutine>
#include <exception>
#include <utility>

namespace coroutines {

// Coroutine return type for a pull-style integer sequence generator.
//
// The coroutine produces values one at a time with co_yield. The caller
// advances to the next value with next(), then reads it with get_current_value().
// The coroutine is also iterable with a range-based for loop via begin/end.
//
// promise_type hooks
// initial_suspend   suspend_always  - starts suspended; caller drives the first step
// final_suspend     suspend_always  - stays suspended at co_return so done() is observable
// return_void       yes
// return_value      no
// yield_value       suspend_always  - suspends after each co_yield, storing the value
struct Generator {
    // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
    struct promise_type {
        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() {
            return {};
        }

        std::suspend_always final_suspend() noexcept {
            return {};
        }

        void return_void() {}

        void unhandled_exception() {
            std::terminate();
        }

        // Store the yielded value and suspend until the caller calls next().
        std::suspend_always yield_value(const int value) {
            _current_value = value;
            return {};
        }

        // The most recently yielded value, readable via get_current_value().
        int _current_value{};
    };
    // NOLINTEND(readability-convert-member-functions-to-static)

    explicit Generator(std::coroutine_handle<promise_type> handle) : _handle(handle) {}

    Generator(const Generator &) = delete;
    Generator &operator=(const Generator &) = delete;

    Generator(Generator &&other) noexcept : _handle(std::exchange(other._handle, {})) {}

    Generator &operator=(Generator &&other) noexcept {
        if (this != &other) {
            // Destroy the current frame before taking ownership of the new one.
            destroy();
            _handle = std::exchange(other._handle, {});
        }
        return *this;
    }

    ~Generator() {
        destroy();
    }

    // Advance to the next yielded value. Returns true if a value is ready, false if the sequence is done.
    // NOLINTNEXTLINE(readability-make-member-function-const) -- resume() mutates coroutine state
    bool next() {
        if (_handle.done()) {
            // The sequence is exhausted.
            return false;
        }
        _handle.resume();
        return !_handle.done();
    }

    // Return the value produced by the most recent co_yield.
    [[nodiscard]] int get_current_value() const {
        return _handle.promise()._current_value;
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

// Support range-based for loops over a Generator.
inline GeneratorIterator<Generator> begin(Generator &generator) {
    return GeneratorIterator{generator};
}

inline GeneratorIterator<Generator> end(const Generator &) {
    return {};
}

}
