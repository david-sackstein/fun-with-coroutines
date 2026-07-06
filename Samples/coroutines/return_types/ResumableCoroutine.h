#pragma once

// Used in Demo 02 - parser.
// Appropriate because the coroutine must be driven from outside:
// it suspends waiting for input and the caller resumes it each time data arrives.

#include <coroutine>
#include <exception>
#include <utility>

namespace coroutines {

// Coroutine return type for a manually driven coroutine.
//
// The coroutine starts suspended and is advanced by the caller via resume().
// At co_return it suspends again (final_suspend = suspend_always), keeping the
// frame alive so the caller can observe done(). The frame is destroyed when
// the ResumableCoroutine object is destroyed or move-assigned.
//
// promise_type hooks
// initial_suspend   suspend_always  - starts suspended; caller drives the first step
// final_suspend     suspend_always  - stays suspended at co_return so done() is observable
// return_void       yes
// return_value      no
// yield_value       no
struct ResumableCoroutine {
    // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
    struct promise_type {
        ResumableCoroutine get_return_object() {
            return ResumableCoroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
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
    };
    // NOLINTEND(readability-convert-member-functions-to-static)

    explicit ResumableCoroutine(std::coroutine_handle<promise_type> handle) : _handle(handle) {}

    // Default constructor supports a ResumableCoroutine that is assigned later.
    ResumableCoroutine() = default;

    ResumableCoroutine(const ResumableCoroutine &) = delete;
    ResumableCoroutine &operator=(const ResumableCoroutine &) = delete;

    ResumableCoroutine(ResumableCoroutine &&other) noexcept : _handle(std::exchange(other._handle, {})) {}

    ResumableCoroutine &operator=(ResumableCoroutine &&other) noexcept {
        if (this != &other) {
            // Destroy the current frame before taking ownership of the new one.
            destroy();
            _handle = std::exchange(other._handle, {});
        }
        return *this;
    }

    ~ResumableCoroutine() {
        destroy();
    }

    // Advance the coroutine by one step; no-op if already done.
    void resume() {
        if (_handle && !_handle.done()) {
            _handle.resume();
        }
    }

    // Return true if the coroutine has reached co_return or the handle is empty.
    [[nodiscard]] bool done() const {
        return !_handle || _handle.done();
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
