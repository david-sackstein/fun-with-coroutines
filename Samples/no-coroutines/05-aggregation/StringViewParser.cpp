#include "no-coroutines/05-aggregation/StringViewParser.h"

namespace no_coroutines {

// Strip leading spaces, find the next space delimiter, and return the token before it.
std::optional<std::string_view> StringViewParser::take_word(std::string_view &input) {
    skip_whitespace(input);
    if (input.empty()) {
        // No non-space content remains.
        return std::nullopt;
    }

    const size_t space = input.find(' ');
    if (space == std::string_view::npos) {
        // No more spaces. The rest of the input is the final token.
        const std::string_view word = input;
        input = {};
        return word;
    }

    // Extract the token before the space and advance past it.
    const std::string_view word = input.substr(0, space);
    input.remove_prefix(space + 1);
    return word;
}

// Remove leading space characters by advancing the start of the view.
void StringViewParser::skip_whitespace(std::string_view &input) {
    while (!input.empty() && input.front() == ' ') {
        input.remove_prefix(1);
    }
}

}
