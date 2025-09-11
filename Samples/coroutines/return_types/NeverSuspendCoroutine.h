#pragma once

#include <coroutine>
#include <exception>

namespace coroutines {

// Coroutine return type for a fire-and-forget coroutine.
//
// The coroutine runs to completion immediately on creation and destroys its own
// frame at co_return (final_suspend = suspend_never). No resume, done check,
// or result retrieval is needed. The returned object is a no-op placeholder.
//
// promise_type hooks
// initial_suspend   suspend_never  — runs immediately on construction
// final_suspend     suspend_never  — destroys its own frame at co_return
// return_void       yes
// return_value      no
// yield_value       no
struct NeverSuspendCoroutine {
    struct promise_type {
        NeverSuspendCoroutine get_return_object() {
            return NeverSuspendCoroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // NOLINTBEGIN(readability-convert-member-functions-to-static) -- coroutine promise_type
        std::suspend_never initial_suspend() {
            return {};
        }
        std::suspend_never final_suspend() noexcept {
            return {};
        }

        void return_void() {
        }

        void unhandled_exception() {
            std::terminate();
        }
        // NOLINTEND(readability-convert-member-functions-to-static)
    };

    explicit NeverSuspendCoroutine(std::coroutine_handle<promise_type> handle) : _handle(handle) {}

    NeverSuspendCoroutine(const NeverSuspendCoroutine &) = delete;
    NeverSuspendCoroutine &operator=(const NeverSuspendCoroutine &) = delete;

    // Non-movable: the frame is destroyed by the coroutine itself; no ownership transfer is needed.
    NeverSuspendCoroutine(NeverSuspendCoroutine &&) = delete;
    NeverSuspendCoroutine &operator=(NeverSuspendCoroutine &&) = delete;

private:
    // Kept as a placeholder; the frame is already destroyed before this object is used.
    std::coroutine_handle<promise_type> _handle;
};

}
