#include "common/async_io/CalcLine.h"
#include "common/pipe/Pipe.h"
#include "common/reactor/Reactor.h"
#include "common/testing/Delays.h"
#include "coroutines/4. async_io/async/AsyncIo.h"
#include "coroutines/4. async_io/calc/CalcServer.h"
#include "coroutines/5. aggregation/CalculatorBatch.h"
#include "coroutines/common/Task.h"

#include <array>
#include <stdexcept>
#include <string>
#include <thread>

#include <unistd.h>

namespace coroutines {

namespace {

constexpr std::array<std::string_view, 4> quiz_questions = {"2+2\n", "10-3\n", "4*5\n", "20/4\n"};

int parse_result_line(std::string_view line) {
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
        line.remove_suffix(1);
    }
    return std::stoi(std::string{line});
}

}

Task<std::array<int, 4>> solve_quiz(Reactor &reactor, const int write_fd, const int read_fd) {
    std::array<int, 4> answers{};

    char response[64];

    for (size_t index = 0; index < quiz_questions.size(); ++index) {
        const std::string_view question = quiz_questions[index];
        const std::string expected = async_io::CalcLine::eval(question);

        const size_t written =
            co_await async_write_exact(reactor, write_fd, {question.data(), question.size()});
        if (written < question.size()) {
            throw std::runtime_error("solve_quiz: write incomplete");
        }

        const size_t received =
            co_await async_read_exact(reactor, read_fd, {response, expected.size()});
        if (received < expected.size()) {
            throw std::runtime_error("solve_quiz: read incomplete");
        }

        if (std::string_view{response, expected.size()} != expected) {
            throw std::runtime_error("solve_quiz: response mismatch");
        }

        answers[index] = parse_result_line(expected);
    }

    close(write_fd);
    co_return answers;
}

std::array<int, 4> run_calculator_batch() {
    const Pipe pipe_client_to_server;
    const Pipe pipe_server_to_client;

    Reactor reactor;

    const CalcServer server(reactor, pipe_client_to_server.read_fd(), pipe_server_to_client.write_fd());
    [[maybe_unused]] const auto server_task = server.run();

    Task<std::array<int, 4>> quiz =
        solve_quiz(reactor, pipe_client_to_server.write_fd(), pipe_server_to_client.read_fd());
    quiz.start();

    std::thread stopper([&reactor] {
        std::this_thread::sleep_for(testing_delay::reactor_safety);
        reactor.stop();
    });
    stopper.detach();

    reactor.run();
    return quiz.get();
}

}
