#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>

namespace parser {

inline constexpr std::array<std::string_view, 4> script_lines = {
    "SET name Alice",
    "GET name",
    "SET age 30",
    "GET age",
};

inline void skip_whitespace(std::string_view& input) {
    while (!input.empty() && input.front() == ' ') {
        input.remove_prefix(1);
    }
}

inline std::optional<std::string_view> take_word(std::string_view& input) {
    skip_whitespace(input);
    if (input.empty()) {
        return std::nullopt;
    }

    const size_t space = input.find(' ');
    if (space == std::string_view::npos) {
        const std::string_view word = input;
        input = {};
        return word;
    }

    const std::string_view word = input.substr(0, space);
    input.remove_prefix(space + 1);
    return word;
}

}
