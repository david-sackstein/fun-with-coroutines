#pragma once

// Callback-based equivalent of the coroutines solve_quiz Task.
// Sends each question one at a time, reads and verifies the answer, then
// advances to the next question. Calls on_complete with all answers when done.
// Compare with the coroutines version where the same logic is a plain loop.

#include "common/reactor/Reactor.h"
#include "common/reactor/WorkGuard.h"

#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace no_coroutines {

class SolveQuiz {
public:
    SolveQuiz(
        Reactor &reactor,
        int write_fd, int read_fd,
        std::span<const std::string_view> questions);

    void start(std::function<void(std::vector<int>)> on_complete);

private:
    void ask_question();
    void async_write_question();
    void on_write_complete(size_t expected, size_t actual);
    void async_read_response();
    void on_read_complete(size_t expected, size_t actual);

    Reactor &_reactor;
    int _write_fd;
    int _read_fd;
    std::span<const std::string_view> _questions;

    size_t _question_index = 0;
    size_t _question_size = 0;
    std::vector<int> _answers;
    char _write_buffer[64]{};
    char _read_buffer[64]{};
    std::string _expected_response;
    std::unique_ptr<WorkGuard> _work_guard;
    std::function<void(std::vector<int>)> _on_complete;
};

}
