#pragma once

#include "common/parser/InputBuffer.h"

#include <coroutine>
#include <optional>
#include <string>

namespace coroutines {

// Custom awaitable for co_await. Suspends the parser coroutine until the buffer
// holds a complete word, then returns that word as the result of co_await.
struct WordAwaitable {
    parser::InputBuffer &buffer;

    // Return true if the buffer already has a complete word, skipping suspension.
    [[nodiscard]] bool await_ready() const {
        return buffer.word_ready();
    }

    // No action on suspend: the driver will resume the coroutine after feeding more input.
    void await_suspend(std::coroutine_handle<> /*handle*/) const {
    }

    // Extract and return the next complete word, or nullopt if input ended.
    [[nodiscard]] std::optional<std::string> await_resume() const {
        return buffer.take_word();
    }
};

}
