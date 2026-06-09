#include "common/io/print.h"
#include "coroutines/02-parser/ProtocolParser.h"

namespace coroutines {

void ProtocolParser::run() {
    io::print("=== Parser protocol demo ===\n");
    io::print("Commands: SET <key> <value>, GET <key>\n\n");
    run_script_sync();
}

void ProtocolParser::run_script_sync() {
    [[maybe_unused]] auto script = run_script();
}

NeverSuspendCoroutine ProtocolParser::run_script() {
    for (const std::string_view line : parser::script_lines) {
        parse_line(line);
    }
    co_return;
}

NeverSuspendCoroutine ProtocolParser::parse_line(const std::string_view line) {
    std::string_view rest = line;
    const auto command = parser::take_word(rest);
    if (!command) {
        co_return;
    }

    if (*command == "SET") {
        handle_set(rest);
        co_return;
    }

    if (*command == "GET") {
        handle_get(rest);
    }
}

NeverSuspendCoroutine ProtocolParser::handle_set(const std::string_view line) {
    std::string_view rest = line;
    const auto key = parser::take_word(rest);
    const auto value = parser::take_word(rest);
    if (!key || !value) {
        co_return;
    }
    _store.emplace(std::string(*key), std::string(*value));
    io::print("OK\n");
}

NeverSuspendCoroutine ProtocolParser::handle_get(const std::string_view line) {
    std::string_view rest = line;
    const auto key = parser::take_word(rest);
    if (!key) {
        co_return;
    }
    const auto iterator = _store.find(std::string(*key));
    if (iterator != _store.end()) {
        io::print("{}\n", iterator->second);
    }
}

}
