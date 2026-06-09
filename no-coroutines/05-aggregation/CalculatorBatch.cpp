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

namespace {

constexpr std::array<std::string_view, 4> quiz_questions = {"2+2\n", "10-3\n", "4*5\n", "20/4\n"};

int parse_result_line(std::string_view line) {
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
        line.remove_suffix(1);
    }
    return std::stoi(std::string{line});
}

class SolveQuiz {
public:
    SolveQuiz(Reactor &reactor, const int write_fd, const int read_fd)
        : _reactor(reactor), _write_fd(write_fd), _read_fd(read_fd) {}

    void start(std::function<void(std::array<int, 4>)> on_complete) {
        _on_complete = std::move(on_complete);
        _work_guard = std::make_unique<WorkGuard>(_reactor);
        ask_question();
    }

private:
    void ask_question() {
        const std::string_view question = quiz_questions[_question_index];
        _question_size = question.size();
        std::copy(question.begin(), question.end(), _write_buffer);
        _expected_response = async_io::CalcLine::eval(question);
        async_write_question();
    }

    void async_write_question() {
        post_write(_reactor, _write_fd, _write_buffer, _question_size,
                   [this](const size_t actual) { on_write_complete(_question_size, actual); });
    }

    void on_write_complete(const size_t expected, const size_t actual) {
        if (actual < expected) {
            throw std::runtime_error(std::format("solve_quiz: write incomplete ({} of {})", actual, expected));
        }

        async_read_response();
    }

    void async_read_response() {
        post_read(_reactor, _read_fd, _read_buffer, _expected_response.size(),
                  stop_at_exact_bytes(),
                  [this](const size_t actual) { on_read_complete(_expected_response.size(), actual); });
    }

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
            ask_question();
            return;
        }

        close(_write_fd);
        _work_guard.reset();
        auto callback = std::move(_on_complete);
        callback(_answers);
    }

    Reactor &_reactor;
    int _write_fd;
    int _read_fd;

    size_t _question_index = 0;
    size_t _question_size = 0;
    std::array<int, 4> _answers{};
    char _write_buffer[64]{};
    char _read_buffer[64]{};
    std::string _expected_response;
    std::unique_ptr<WorkGuard> _work_guard;
    std::function<void(std::array<int, 4>)> _on_complete;
};

}

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

    std::thread stopper([&reactor] {
        std::this_thread::sleep_for(testing_delay::reactor_safety);
        reactor.stop();
    });
    stopper.detach();

    reactor.run();
    return answers;
}

}
