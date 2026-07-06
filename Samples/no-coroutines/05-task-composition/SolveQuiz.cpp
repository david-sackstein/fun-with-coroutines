#include "common/async_io/CalcLine.h"
#include "common/reactor/Reactor.h"
#include "no-coroutines/04-async-io/async/AsyncIo.h"
#include "no-coroutines/05-task-composition/SolveQuiz.h"

#include <format>
#include <stdexcept>
#include <string>
#include <string_view>

#include <unistd.h>

namespace no_coroutines {

int parse_result_line(std::string_view line);

SolveQuiz::SolveQuiz(
    Reactor &reactor,
    const int write_fd, const int read_fd,
    const std::span<const std::string_view> questions)
    : _reactor(reactor), _write_fd(write_fd), _read_fd(read_fd),
      _questions(questions), _answers(questions.size()) {}

// Acquire the WorkGuard and begin the first question.
void SolveQuiz::start(std::function<void(std::vector<int>)> on_complete) {
    _on_complete = std::move(on_complete);
    _work_guard = std::make_unique<WorkGuard>(_reactor);
    ask_question();
}

// Copy the current question into the write buffer and pre-compute the expected response.
void SolveQuiz::ask_question() {
    const std::string_view question = _questions[_question_index];
    _question_size = question.size();
    std::copy(question.begin(), question.end(), _write_buffer);
    _expected_response = async_io::CalcLine::eval(question);
    async_write_question();
}

// Post the async write of the current question to the server pipe.
void SolveQuiz::async_write_question() {
    post_write(_reactor, _write_fd, _write_buffer, _question_size,
               [this](const size_t actual) { on_write_complete(_question_size, actual); });
}

// Verify the write completed and kick off the async response read.
void SolveQuiz::on_write_complete(const size_t expected, const size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format("solve_quiz: write incomplete ({} of {})", actual, expected));
    }
    async_read_response();
}

// Post the async read of the expected number of response bytes.
void SolveQuiz::async_read_response() {
    post_read(_reactor, _read_fd, _read_buffer, _expected_response.size(),
              stop_at_exact_bytes(),
              [this](const size_t actual) { on_read_complete(_expected_response.size(), actual); });
}

// Record the answer and advance to the next question or finish.
void SolveQuiz::on_read_complete(const size_t expected, const size_t actual) {
    if (actual < expected) {
        throw std::runtime_error(std::format("solve_quiz: read incomplete ({} of {})", actual, expected));
    }

    _answers[_question_index] = parse_result_line({_read_buffer, actual});

    ++_question_index;
    if (_question_index < _questions.size()) {
        ask_question();
        return;
    }

    // All questions answered. Signal the server, release work, and deliver results.
    close(_write_fd);
    _work_guard.reset();
    auto callback = std::move(_on_complete);
    callback(std::move(_answers));
}

// Strip a trailing newline and parse the result as an integer.
int parse_result_line(const std::string_view line) {
    std::string_view trimmed = line;
    while (!trimmed.empty() && (trimmed.back() == '\n' || trimmed.back() == '\r')) {
        trimmed.remove_suffix(1);
    }
    return std::stoi(std::string{trimmed});
}

}
