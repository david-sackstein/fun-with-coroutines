#pragma once

#include "common/generator/Iterator.h"

#include <coroutine>

namespace coroutines {
    struct Generator {

        struct promise_type {
            Generator get_return_object() {
                return Generator(std::coroutine_handle<promise_type>::from_promise(*this));
            }

            // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
            std::suspend_always initial_suspend() { return {}; }

            std::suspend_always final_suspend() noexcept { return {}; }

            void return_void() {}

            void unhandled_exception() {}
            // NOLINTEND(readability-convert-member-functions-to-static)

            std::suspend_always yield_value(const int value) {
                current_value = value;
                return {};
            }

            int current_value;
        };

        explicit Generator(std::coroutine_handle<promise_type> handle) :
            _handle(handle) {
        }

        Generator(Generator&& other) noexcept : _handle{} {
            swap(*this, other);
        }

        Generator& operator=(Generator other) noexcept {
            swap(*this, other);
            return *this;
        }

        Generator(const Generator&) = delete;

        friend void swap(Generator& lhs, Generator& rhs) noexcept {
            std::swap(lhs._handle, rhs._handle);
        }

        ~Generator() {
            if (_handle) {
                _handle.destroy();
            }
        }

        // NOLINTNEXTLINE(readability-make-member-function-const) -- resume() mutates coroutine state
        bool next() {
            if (_handle.done()) {
                return false;
            }
            _handle.resume();
            return !_handle.done();
        }

        [[nodiscard]] int get_current_value() const {
            const auto &promise = _handle.promise();
            return promise.current_value;
        }

        std::coroutine_handle<promise_type> _handle;
    };

    // Enable range-based for loop via ADL
    inline GeneratorIterator<Generator> begin(Generator& generator) {
        return GeneratorIterator(generator);
    }

    inline GeneratorIterator<Generator> end(const Generator&) {
        return {};
    }
}