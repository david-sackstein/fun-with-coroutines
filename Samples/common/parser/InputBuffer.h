#pragma once

// Streaming tokenizer for push-style input.
//
// Input arrives in arbitrary-sized chunks via append().
// Tokens are delimited by space or newline.
// Call finish() when no more input is coming; this allows the last
// token without a trailing delimiter to be extracted.
//
// Example:
//   buffer.append("SET na");
//   buffer.append("me Alice\n");
//   buffer.take_word();  ->  "SET"
//   buffer.take_word();  ->  "name"
//   buffer.take_word();  ->  "Alice"

#include <optional>
#include <string>
#include <string_view>

namespace parser {

class InputBuffer {
public:
    // Append a chunk of input; may be any size including partial tokens.
    void append(std::string_view chunk);

    // Signal that no more input is coming.
    void finish();

    // Return true if the buffer has been marked finished.
    [[nodiscard]] bool eof() const;

    // Return true if a complete token is available for take_word().
    [[nodiscard]] bool word_ready() const;

    // Return true when the buffer is finished and all tokens have been consumed.
    [[nodiscard]] bool at_end() const;

    // Remove and return the next complete token, or nullopt if none is ready.
    std::optional<std::string> take_word();

private:
    // Return the index of the first non-whitespace character in _data.
    [[nodiscard]] size_t leading_content_index() const;

    // Return the position of the first space or newline at or after start, or npos.
    [[nodiscard]] size_t find_delimiter(size_t start) const;

    // Drop leading whitespace characters from _data in place.
    void skip_whitespace();

    [[nodiscard]] static bool is_whitespace(char character);

    std::string _data;
    bool _eof = false;
};

}
