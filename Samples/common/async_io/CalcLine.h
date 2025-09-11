#pragma once

// Evaluates a single-line arithmetic expression of the form "left op right\n".
// Supported operators: + - * /
// Returns the result as a string followed by a newline, or "invalid syntax\n".
//
// Example:
//   CalcLine::eval("3 + 4\n")  ->  "7\n"
//   CalcLine::eval("10 / 0\n") ->  "invalid syntax\n"

#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace async_io {

class CalcLine {
public:
    // Parse line, evaluate the expression, and return the result string.
    [[nodiscard]] static std::string eval(const std::string_view line) {
        std::istringstream input{std::string{line}};
        int left = 0;
        int right = 0;
        char op = '\0';

        if (!(input >> left >> op >> right)) {
            // The line does not contain two integers and an operator.
            return "invalid syntax\n";
        }
        if (op != '+' && op != '-' && op != '*' && op != '/') {
            // The operator is not one of the four supported operations.
            return "invalid syntax\n";
        }
        input >> std::ws;
        if (input.peek() != std::char_traits<char>::eof()) {
            // Extra characters follow the expression.
            return "invalid syntax\n";
        }

        const std::optional<int> result = apply_operator(op, left, right);
        if (!result) {
            // Division by zero or unrecognised operator.
            return "invalid syntax\n";
        }

        return std::to_string(*result) + '\n';
    }

private:
    // Apply op to left and right; return nullopt for division by zero.
    static std::optional<int> apply_operator(const char op, const int left, const int right) {
        switch (op) {
            case '+': return left + right;
            case '-': return left - right;
            case '*': return left * right;
            case '/':
                if (right == 0) {
                    // Division by zero is not defined.
                    return std::nullopt;
                }
                return left / right;
            default:
                return std::nullopt;
        }
    }
};

}
