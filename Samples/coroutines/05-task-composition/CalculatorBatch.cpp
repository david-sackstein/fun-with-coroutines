#include "common/async_io/CalcLine.h"
#include "common/pipe/Pipe.h"
#include "common/reactor/Reactor.h"
#include "common/reactor/ReactorStopper.h"
#include "common/reactor/WorkGuard.h"
#include "common/testing/Delays.h"
#include "coroutines/04-async-io/async/AsyncIo.h"
#include "coroutines/04-async-io/calc/CalcServer.h"
#include "coroutines/05-task-composition/CalculatorBatch.h"
#include "coroutines/return_types/task/Task.h"

#include <array>
#include <format>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <unistd.h>

namespace coroutines {

Task<std::vector<int>> solve_quiz(
    Reactor &reactor,
    int write_fd, int read_fd,
    std::span<const std::string_view> questions);
int parse_result_line(std::string_view line);

// Wire the server and solve_quiz together, run the Reactor, and return the collected answers.
std::vector<int> run_calculator_batch() {
    constexpr std::array<std::string_view, 4> quiz_questions = {"2+2\n", "10-3\n", "4*5\n", "20/4\n"};

    const Pipe pipe_client_to_server;
    const Pipe pipe_server_to_client;

    Reactor reactor;

    const CalcServer server(reactor, pipe_client_to_server.read_fd(), pipe_server_to_client.write_fd());
    const auto server_coro = server.run();

    auto task = solve_quiz(
        reactor,
        pipe_client_to_server.write_fd(), pipe_server_to_client.read_fd(),
        quiz_questions);

    task.start();

    const ReactorStopper stopper(reactor, testing_delay::reactor_safety);

    reactor.run();

    return task.result();
}

// Coroutine that sends each question, reads and verifies the response, and returns all answers.
// The WorkGuard keeps the Reactor alive; releasing it at co_return lets the Reactor stop.
Task<std::vector<int>> solve_quiz(
    Reactor &reactor,
    const int write_fd, const int read_fd,
    const std::span<const std::string_view> questions) {
    const WorkGuard guard(reactor);

    std::vector<int> answers(questions.size());
    char write_buffer[64]{};
    char read_buffer[64]{};

    for (size_t i = 0; i < questions.size(); ++i) {
        const std::string_view question = questions[i];
        std::ranges::copy(question, write_buffer);
        const std::string expected = async_io::CalcLine::eval(question);

        const size_t written =
            co_await async_write_exact(reactor, write_fd, {write_buffer, question.size()});

        if (written < question.size()) {
            throw std::runtime_error(
                std::format("solve_quiz: write incomplete ({} of {})", written, question.size()));
        }

        const size_t received =
            co_await async_read_exact(reactor, read_fd, {read_buffer, expected.size()});

        if (received < expected.size()) {
            throw std::runtime_error(
                std::format("solve_quiz: read incomplete ({} of {})", received, expected.size()));
        }

        if (std::string_view{read_buffer, received} != expected) {
            throw std::runtime_error("solve_quiz: response mismatch");
        }

        answers[i] = parse_result_line(expected);
    }

    // Signal EOF to the server so its read loop exits and its WorkGuard drops.
    close(write_fd);
    co_return answers;
}

// Strip a trailing newline and parse the result as an integer.
int parse_result_line(std::string_view line) {
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
        line.remove_suffix(1);
    }
    return std::stoi(std::string{line});
}

}
