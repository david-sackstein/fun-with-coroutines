#include "common/async_io/CalcLine.h"
#include "common/pipe/Pipe.h"
#include "common/reactor/Reactor.h"
#include "common/testing/Delays.h"
#include "coroutines/04-async-io/async/AsyncIo.h"
#include "coroutines/04-async-io/calc/CalcServer.h"
#include "coroutines/05-aggregation/CalculatorBatch.h"
#include "coroutines/return_types/Task.h"

#include <array>
#include <stdexcept>
#include <string>
#include <thread>

#include <unistd.h>

namespace coroutines {

// The four arithmetic questions sent to the server in order.
constexpr std::array<std::string_view, 4> quiz_questions = {"2+2\n", "10-3\n", "4*5\n", "20/4\n"};

// Strip a trailing newline and parse the result as an integer.
int parse_result_line(std::string_view line) {
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
        line.remove_suffix(1);
    }
    return std::stoi(std::string{line});
}

// For each question: write it to the server, read back the response, verify it, record the answer.
Task<std::array<int, 4>> solve_quiz(Reactor &reactor, const int write_fd, const int read_fd) {
    std::array<int, 4> answers{};
    char response[64];

    for (size_t index = 0; index < quiz_questions.size(); ++index) {
        const std::string_view question = quiz_questions[index];

        // Pre-compute the expected answer to know how many bytes to read back.
        const std::string expected = async_io::CalcLine::eval(question);

        // Suspend until all question bytes have been written to the server pipe.
        const size_t written =
            co_await async_write_exact(reactor, write_fd, {question.data(), question.size()});

        if (written < question.size()) {
            throw std::runtime_error("solve_quiz: write incomplete");
        }

        // Suspend until all response bytes have been received from the server pipe.
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

    // Signal to the server that no more questions are coming.
    close(write_fd);
    co_return answers;
}

// Wire the server and quiz together, run the Reactor, and return the collected answers.
std::array<int, 4> run_calculator_batch() {
    const Pipe pipe_client_to_server;
    const Pipe pipe_server_to_client;

    Reactor reactor;

    // Start the server coroutine; it runs eagerly via FinalSuspendCoroutine.
    const CalcServer server(reactor, pipe_client_to_server.read_fd(), pipe_server_to_client.write_fd());
    [[maybe_unused]] const auto server_task = server.run();

    // Start the quiz coroutine from its initial suspension point.
    Task<std::array<int, 4>> quiz =
        solve_quiz(reactor, pipe_client_to_server.write_fd(), pipe_server_to_client.read_fd());
    quiz.start();

    // Safety thread: stop the Reactor if it is still running after the timeout.
    std::thread stopper([&reactor] {
        std::this_thread::sleep_for(testing_delay::reactor_safety);
        reactor.stop();
    });
    stopper.detach();

    reactor.run();
    return quiz.get();
}

}
