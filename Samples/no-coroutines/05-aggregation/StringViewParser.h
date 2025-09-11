#pragma once

// Synchronous tokenizer for complete, in-memory string_view lines.
// Unlike InputBuffer, this parser does not handle streaming input; the full
// line must already be present. Used in demo 05 to parse script_lines.

#include <optional>
#include <string_view>

namespace no_coroutines {

class StringViewParser {
public:
    // Remove leading spaces, then extract and return the next space-delimited token.
    // Advances input past the token and its trailing space. Returns nullopt if input is empty.
    static std::optional<std::string_view> take_word(std::string_view &input);

private:
    // Remove leading space characters from input in place.
    static void skip_whitespace(std::string_view &input);
};

}
