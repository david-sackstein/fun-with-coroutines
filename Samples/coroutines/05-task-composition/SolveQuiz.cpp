#include "common/reactor/Reactor.h"
#include "coroutines/04-async-io/async/AsyncIo.h"
#include "coroutines/05-task-composition/SolveQuiz.h"
#include "coroutines/return_types/task/Task.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <unistd.h>

namespace coroutines {

// Coroutine that sends each question, reads the response, accumulates and returns the answers
Task<std::vector<int>> solve_quiz(
    Reactor &reactor,
    const int write_fd, const int read_fd,
    const std::span<const std::string_view> questions) {

    std::vector<int> answers;
    answers.reserve(questions.size());

    char write_buffer[64]{};
    char read_buffer[64]{};

    for (const std::string_view question : questions) {
        std::ranges::copy(question, write_buffer);

        co_await async_write_exact(reactor, write_fd, {write_buffer, question.size()});

        const size_t received =
            co_await async_read_until<'\n'>(reactor, read_fd, read_buffer);

        answers.push_back(std::stoi(std::string{read_buffer, received}));
    }

    close(write_fd);
    co_return answers;
}

}
