#include "common/async_io/CalcLine.h"
#include "common/pipe/Pipe.h"
#include "common/reactor/Reactor.h"
#include "common/testing/Delays.h"
#include "no-coroutines/04-async-io/async/AsyncIo.h"
#include "no-coroutines/04-async-io/calc/CalcServer.h"
#include "no-coroutines/05-aggregation/CalculatorBatch.h"

#include <algorithm>
#include <array>
#include <format>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include <unistd.h>

namespace no_coroutines {

// The four arithmetic questions sent to the server in order.
constexpr std::array<std::string_view, 4> quiz_questions = {"2+2\n", "10-3\n", "4*5\n", "20/4\n"};

// Strip a trailing newline and parse the result as an integer.
int parse_result_line(std::string_view line) {
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
        line.remove_suffix(1);
    }
    return std::stoi(std::string{line});
}

// Callback-based equivalent of the coroutines solve_quiz Task.
// Sends each question one at a time, reads and verifies the answer, then
// advances to the next question. Calls on_complete with all answers when done.
class SolveQuiz {
public:
    SolveQuiz(Reactor &reactor, const int write_fd, const int read_fd)
        : _reactor(reactor), _write_fd(write_fd), _read_fd(read_fd) {}

    // Acquire the WorkGuard and begin the first question.
    void start(std::function<void(std::array<int, 4>)> on_complete) {
        _on_complete = std::move(on_complete);
        _work_guard = std::make_unique<WorkGuard>(_reactor);
        ask_question();
    }

private:
    // Copy the current question into the write buffer and pre-compute the expected response.
    void ask_question() {
        const std::string_view question = quiz_questions[_question_index];
        _question_size = question.size();
        std::copy(question.begin(), question.end(), _write_buffer);
        _expected_response = async_io::CalcLine::eval(question);
        async_write_question();
    }

    // Post the async write of the current question to the server pipe.
    void async_write_question() {
        post_write(_reactor, _write_fd, _write_buffer, _question_size,
                   [this](const size_t actual) { on_write_complete(_question_size, actual); });
    }

    // Verify the write completed and kick off the async response read.
    void on_write_complete(const size_t expected, const size_t actual) {
        if (actual < expected) {
            throw std::runtime_error(std::format("solve_quiz: write incomplete ({} of {})", actual, expected));
        }
        async_read_response();
    }

    // Post the async read of the expected number of response bytes.
    void async_read_response() {
        post_read(_reactor, _read_fd, _read_buffer, _expected_response.size(),
                  stop_at_exact_bytes(),
                  [this](const size_t actual) { on_read_complete(_expected_response.size(), actual); });
    }

    // Verify the response, record the answer, and advance to the next question or finish.
    void on_read_complete(const size_t expected, const size_t actual) {
        if (actual < expected) {
            throw std::runtime_error(std::format("solve_quiz: read incomplete ({} of {})", actual, expected));
        }

        if (std::string_view{_read_buffer, expected} != _expected_response) {
            throw std::runtime_error("solve_quiz: response mismatch");
        }

        _answers[_question_index] = parse_result_line(_expected_response);

        ++_question_index;
        if (_question_index < quiz_questions.size()) {
            // More questions remain. Start the next one.
            ask_question();
            return;
        }

        // All questions answered. Signal the server, release work, and deliver results.
        close(_write_fd);
        _work_guard.reset();
        auto callback = std::move(_on_complete);
        callback(_answers);
    }

    Reactor &_reactor;
    int _write_fd;
    int _read_fd;

    size_t _question_index = 0;   // index of the question currently being asked
    size_t _question_size = 0;    // byte length of the current question
    std::array<int, 4> _answers{};
    char _write_buffer[64]{};     // holds the current question bytes
    char _read_buffer[64]{};      // holds the server response bytes
    // Pre-computed expected response for the current question, held across the async read.
    std::string _expected_response;
    std::unique_ptr<WorkGuard> _work_guard;
    std::function<void(std::array<int, 4>)> _on_complete;
};

// Wire the server and SolveQuiz together, run the Reactor, and return the collected answers.
std::array<int, 4> run_calculator_batch() {
    const Pipe pipe_client_to_server;
    const Pipe pipe_server_to_client;

    Reactor reactor;

    CalcServer server(reactor, pipe_client_to_server.read_fd(), pipe_server_to_client.write_fd());
    server.run();

    std::array<int, 4> answers{};
    const auto quiz = std::make_shared<SolveQuiz>(reactor, pipe_client_to_server.write_fd(),
                                                   pipe_server_to_client.read_fd());
    quiz->start([&reactor, &answers](const std::array<int, 4> &result) {
        answers = result;
        reactor.stop();
    });

    // Safety thread: stop the Reactor if it is still running after the timeout.
    std::thread stopper([&reactor] {
        std::this_thread::sleep_for(testing_delay::reactor_safety);
        reactor.stop();
    });
    stopper.detach();

    reactor.run();
    return answers;
}

}
