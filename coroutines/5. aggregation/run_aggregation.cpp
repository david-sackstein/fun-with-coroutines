#include "common/async_io/CalcLine.h"
#include "common/event_loop/EventLoop.h"
#include "common/io/print.h"
#include "common/parser/Protocol.h"
#include "common/pipe/Pipe.h"
#include "common/reactor/Reactor.h"
#include "coroutines/3. async_tasks/AsyncTaskMarshalled.h"
#include "coroutines/4. async_io/async/AsyncIo.h"
#include "coroutines/4. async_io/calc/CalcServer.h"
#include "coroutines/common/Task.h"

#include <array>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

#include <unistd.h>

namespace coroutines {

using namespace std::chrono_literals;

namespace {

constexpr std::array<std::string_view, 4> quiz_questions = {"2+2\n", "10-3\n", "4*5\n", "20/4\n"};

int parse_result_line(std::string_view line) {
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
        line.remove_suffix(1);
    }
    return std::stoi(std::string{line});
}

Task<std::map<std::string, std::string>> load_store_from_script() {
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

    co_return store;
}

Task<int> aggregate_totals(EventLoop &loop) {
    const EventLoop::Work guard(loop);

    int sum = 0;

    co_await AsyncTaskMarshalled{loop};
    sum += 10;

    co_await AsyncTaskMarshalled{loop};
    sum += 20;

    co_await AsyncTaskMarshalled{loop};
    sum += 30;

    co_return sum;
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
        io::print("[Quiz] {} → {}\n", std::string_view{question.data(), question.size() - 1}, answers[index]);
    }

    close(write_fd);
    co_return answers;
}

}

void run_parser_sample() {
    io::print("\n=== Sample 1 — Parser store ===\n");

    const std::map<std::string, std::string> store = load_store_from_script().get();

    io::print("store ({} entries):\n", store.size());
    for (const auto &[key, value] : store) {
        io::print("  {} → {}\n", key, value);
    }
}

void run_async_sample() {
    io::print("\n=== Sample 2 — Marshalled async aggregate ===\n");

    Task<int> aggregate = aggregate_totals(g_loop);
    aggregate.start();
    g_loop.run();

    io::print("sum: {}\n", aggregate.get());
}

void run_calculator_sample() {
    io::print("\n=== Sample 3 — Calculator RPC batch ===\n");

    const Pipe pipe_client_to_server;
    const Pipe pipe_server_to_client;

    Reactor reactor;

    const CalcServer server(reactor, pipe_client_to_server.read_fd(), pipe_server_to_client.write_fd());
    [[maybe_unused]] const auto server_task = server.run();

    Task<std::array<int, 4>> quiz =
        solve_quiz(reactor, pipe_client_to_server.write_fd(), pipe_server_to_client.read_fd());
    quiz.start();

    std::thread stopper([&reactor] {
        std::this_thread::sleep_for(5s);
        reactor.stop();
    });
    stopper.detach();

    try {
        reactor.run();
    } catch (const std::exception &exception) {
        io::print("✗ Error: {}\n", exception.what());
        return;
    }

    const std::array<int, 4> answers = quiz.get();

    io::print("answers:");
    for (const int answer : answers) {
        io::print(" {}", answer);
    }
    io::print("\n");
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
