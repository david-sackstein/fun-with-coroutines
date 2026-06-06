#pragma once

#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace async_io {

namespace detail {

inline void strip_line_ending(std::string_view &line) {
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
        line.remove_suffix(1);
    }
}

inline void skip_whitespace(std::string_view &input) {
    while (!input.empty() && input.front() == ' ') {
        input.remove_prefix(1);
    }
}

inline std::optional<int> take_int(std::string_view &input) {
    skip_whitespace(input);
    if (input.empty()) {
        return std::nullopt;
    }

    int value = 0;
    const auto [next, error_code] = std::from_chars(input.data(), input.data() + input.size(), value);
    if (error_code != std::errc{} || next == input.data()) {
        return std::nullopt;
    }

    input.remove_prefix(static_cast<size_t>(next - input.data()));
    return value;
}

inline std::optional<char> take_operator(std::string_view &input) {
    skip_whitespace(input);
    if (input.empty()) {
        return std::nullopt;
    }

    const char op = input.front();
    if (op != '+' && op != '-' && op != '*' && op != '/') {
        return std::nullopt;
    }

    input.remove_prefix(1);
    return op;
}

inline std::optional<int> apply_operator(const char op, const int left, const int right) {
    switch (op) {
        case '+':
            return left + right;
        case '-':
            return left - right;
        case '*':
            return left * right;
        case '/':
            if (right == 0) {
                return std::nullopt;
            }
            return left / right;
        default:
            return std::nullopt;
    }
}

}

inline std::string eval_binary_expression(std::string_view line) {
    detail::strip_line_ending(line);

    std::string_view rest = line;
    const std::optional<int> left = detail::take_int(rest);
    const std::optional<char> op = detail::take_operator(rest);
    const std::optional<int> right = detail::take_int(rest);

    detail::skip_whitespace(rest);
    if (!left || !op || !right || !rest.empty()) {
        return "invalid syntax\n";
    }

    const std::optional<int> result = detail::apply_operator(*op, *left, *right);
    if (!result) {
        return "invalid syntax\n";
    }

    return std::to_string(*result) + '\n';
}

}
