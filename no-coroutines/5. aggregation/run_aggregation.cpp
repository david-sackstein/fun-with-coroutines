#include "common/async_io/CalcLine.h"
#include "common/event_loop/EventLoop.h"
#include "common/io/print.h"
#include "common/parser/Protocol.h"
#include "common/pipe/Pipe.h"
#include "common/reactor/Reactor.h"
#include "no-coroutines/3. async_tasks/AsyncTaskRunnerMarshalled.h"
#include "no-coroutines/4. async_io/async/AsyncIoCallbacks.h"
#include "no-coroutines/4. async_io/calc/CalcServer.h"

#include <algorithm>
#include <array>
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

#include <unistd.h>

namespace no_coroutines {

using namespace std::chrono_literals;

namespace {

constexpr std::array<std::string_view, 4> quiz_questions = {"2+2\n", "10-3\n", "4*5\n", "20/4\n"};

int parse_result_line(std::string_view line) {
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
        line.remove_suffix(1);
    }
    return std::stoi(std::string{line});
}

std::map<std::string, std::string> load_store_from_script() {
    std::map<std::string, std::string> store;

    for (const std::string_view line : parser::script_lines) {
        std::string_view rest = line;
        const std::optional<std::string_view> command = parser::take_word(rest);
        if (!command) {
            continue;
        }

        if (*command == "SET") {
            const std::optional<std::string_view> key = parser::take_word(rest);
            const std::optional<std::string_view> value = parser::take_word(rest);
            if (key && value) {
                store.emplace(std::string{*key}, std::string{*value});
            }
            continue;
        }

        if (*command == "GET") {
            const std::optional<std::string_view> key = parser::take_word(rest);
            if (!key) {
                continue;
            }
            const auto iterator = store.find(std::string{*key});
            if (iterator != store.end()) {
                io::print("{}\n", iterator->second);
            }
        }
    }

    return store;
}

void aggregate_totals(EventLoop &loop, std::shared_ptr<EventLoop::Work> *keepalive,
                      const std::function<void(int)> &on_complete) {
    const auto runner = std::make_shared<AsyncTaskRunnerMarshalled>(loop);
    const auto sum = std::make_shared<int>(0);

    runner->run_async_operation([runner, keepalive, sum, on_complete] {
        *sum += 10;
        runner->run_async_operation([runner, keepalive, sum, on_complete] {
            *sum += 20;
            runner->run_async_operation([keepalive, sum, on_complete] {
                *sum += 30;
                keepalive->reset();
                on_complete(*sum);
            });
        });
    });
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
        io::print("[Quiz] {} → {}\n",
                  std::string_view{_write_buffer, _question_size - 1},
                  _answers[_question_index]);

        ++_question_index;
        if (_question_index < quiz_questions.size()) {
            ask_question();
            return;
        }

        close(_write_fd);
        _work_guard.reset();
        _on_complete(_answers);
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

void solve_quiz(Reactor &reactor, const int write_fd, const int read_fd,
                std::function<void(std::array<int, 4>)> on_complete) {
    SolveQuiz(reactor, write_fd, read_fd).start(std::move(on_complete));
}

}

void run_parser_sample() {
    io::print("\n=== Sample 1 — Parser store ===\n");

    const std::map<std::string, std::string> store = load_store_from_script();

    io::print("store ({} entries):\n", store.size());
    for (const auto &[key, value] : store) {
        io::print("  {} → {}\n", key, value);
    }
}

void run_async_sample() {
    io::print("\n=== Sample 2 — Marshalled async aggregate ===\n");

    auto keepalive = std::make_shared<EventLoop::Work>(g_loop);
    aggregate_totals(g_loop, &keepalive, [](const int sum) {
        io::print("sum: {}\n", sum);
    });
    g_loop.run();
}

void run_calculator_sample() {
    io::print("\n=== Sample 3 — Calculator RPC batch ===\n");

    const Pipe pipe_client_to_server;
    const Pipe pipe_server_to_client;

    Reactor reactor;

    CalcServer server(reactor, pipe_client_to_server.read_fd(), pipe_server_to_client.write_fd());
    server.run();

    solve_quiz(reactor, pipe_client_to_server.write_fd(), pipe_server_to_client.read_fd(),
               [&reactor](const std::array<int, 4> &answers) {
                   io::print("answers:");
                   for (const int answer : answers) {
                       io::print(" {}", answer);
                   }
                   io::print("\n");
                   reactor.stop();
               });

    std::thread stopper([&reactor] {
        std::this_thread::sleep_for(5s);
        reactor.stop();
    });
    stopper.detach();

    try {
        reactor.run();
    } catch (const std::exception &exception) {
        io::print("✗ Error: {}\n", exception.what());
    }
}

void run_aggregation() {
    io::print("=== Aggregation demo ===\n");
    io::print("Three samples — parser map, async aggregate, calculator RPC batch\n");

    run_parser_sample();
    run_async_sample();
    run_calculator_sample();

    io::print("\n=== Aggregation demo finished ===\n");
}

}
