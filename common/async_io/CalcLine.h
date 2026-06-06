#pragma once

#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace async_io {

class CalcLine {
public:
    [[nodiscard]] static std::string eval(const std::string_view line) {
        std::istringstream input{std::string{line}};
        int left = 0;
        int right = 0;
        char op = '\0';

        if (!(input >> left >> op >> right)) {
            return "invalid syntax\n";
        }
        if (op != '+' && op != '-' && op != '*' && op != '/') {
            return "invalid syntax\n";
        }
        input >> std::ws;
        if (input.peek() != std::char_traits<char>::eof()) {
            return "invalid syntax\n";
        }

        const std::optional<int> result = apply_operator(op, left, right);
        if (!result) {
            return "invalid syntax\n";
        }

        return std::to_string(*result) + '\n';
    }

private:
    static std::optional<int> apply_operator(const char op, const int left, const int right) {
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
};

}
