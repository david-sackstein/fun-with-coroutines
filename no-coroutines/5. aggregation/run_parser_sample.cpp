#include "common/io/print.h"
#include "common/parser/Protocol.h"

#include <map>
#include <optional>
#include <string>

namespace no_coroutines {

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

void run_parser_sample() {
    io::print("\n=== Sample 1 — Parser store ===\n");

    const std::map<std::string, std::string> store = load_store_from_script();

    io::print("store ({} entries):\n", store.size());
    for (const auto &[key, value] : store) {
        io::print("  {} → {}\n", key, value);
    }
}

}
