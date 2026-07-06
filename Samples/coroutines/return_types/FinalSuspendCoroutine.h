#pragma once

// Used in Demo 04 - async I/O.
// Appropriate because the coroutine must outlive its own co_return:
// I/O callbacks hold a handle into the frame, so the owner must destroy it explicitly.

#include <coroutine>
#include <exception>

namespace coroutines {

// Coroutine return type for an async coroutine that runs eagerly and is
// destroyed by its owner.
//
// The coroutine starts running immediately on creation (initial_suspend = suspend_never).
// It suspends at co_return (final_suspend = suspend_always) instead of destroying
// its own frame, so the owning FinalSuspendCoroutine object can destroy it safely.
// The object is non-movable: the frame address must remain stable because I/O
// callbacks hold a coroutine_handle pointing into it.
//
// promise_type hooks
// initial_suspend   suspend_never   - runs immediately on construction, no explicit start needed
// final_suspend     suspend_always  - keeps the frame alive for the owner to destroy
// return_void       yes
// return_value      no
// yield_value       no
struct FinalSuspendCoroutine {
    // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
    struct promise_type {
        FinalSuspendCoroutine get_return_object() {
            return FinalSuspendCoroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() {
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

    explicit FinalSuspendCoroutine(std::coroutine_handle<promise_type> handle) : _handle(handle) {}

    FinalSuspendCoroutine(const FinalSuspendCoroutine &) = delete;
    FinalSuspendCoroutine &operator=(const FinalSuspendCoroutine &) = delete;

    // Non-movable: I/O callbacks hold a handle into the frame; the address must be stable.
    FinalSuspendCoroutine(FinalSuspendCoroutine &&) = delete;
    FinalSuspendCoroutine &operator=(FinalSuspendCoroutine &&) = delete;

    // Destroy the coroutine frame when the owning object is destroyed.
    ~FinalSuspendCoroutine() {
        if (_handle) {
            _handle.destroy();
        }
    }

private:
    std::coroutine_handle<promise_type> _handle;
};

}
