#include "common/io/print.h"
#include "common/parser/Protocol.h"
#include "coroutines/05-aggregation/ParserStore.h"

#include <map>
#include <optional>
#include <string>

namespace coroutines {

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

}
