#include "common/parser/InputBuffer.h"

#include <utility>

namespace parser {

// Grow the internal buffer with new input bytes.
void InputBuffer::append(const std::string_view chunk) {
    _data.append(chunk);
}

// Mark the stream as finished; no further chunks will arrive.
void InputBuffer::finish() {
    _eof = true;
}

// Report whether the stream has been marked finished.
bool InputBuffer::eof() const {
    return _eof;
}

bool InputBuffer::word_ready() const {
    const size_t index = leading_content_index();
    if (index >= _data.size()) {
        // The buffer has no token content yet.
        return false;
    }

    if (find_delimiter(index) != std::string::npos) {
        // A space or newline marks the end of a complete token.
        return true;
    }

    // A trailing partial token is complete only when no more input is coming.
    return _eof;
}

// Report whether the stream is finished and the buffer is empty.
bool InputBuffer::at_end() const {
    return leading_content_index() >= _data.size() && _eof;
}

// Extract and return the next complete token, or nullopt if none is ready.
std::optional<std::string> InputBuffer::take_word() {
    if (!word_ready()) {
        // The next token is incomplete. Wait for more input.
        return std::nullopt;
    }

    skip_whitespace();
    const size_t delimiter = find_delimiter(0);
    if (delimiter != std::string::npos) {
        // A delimiter was found. Extract the token before it and drop the delimiter.
        std::string word = _data.substr(0, delimiter);
        _data.erase(0, delimiter + 1);
        return word;
    }

    // No delimiter, but eof is set. The entire remaining buffer is the final token.
    std::string word = std::exchange(_data, {});
    return word;
}

// Return the index of the first non-whitespace byte in _data.
size_t InputBuffer::leading_content_index() const {
    size_t index = 0;
    while (index < _data.size() && is_whitespace(_data[index])) {
        ++index;
    }
    return index;
}

// Return the position of the earliest space or newline at or after start, or npos.
size_t InputBuffer::find_delimiter(const size_t start) const {
    const size_t space = _data.find(' ', start);
    const size_t newline = _data.find('\n', start);
    if (space == std::string::npos) {
        // No space found. Use newline position (may also be npos).
        return newline;
    }
    if (newline == std::string::npos) {
        // No newline found. Use space position.
        return space;
    }
    // Both found. Return whichever comes first.
    return std::min(space, newline);
}

// Remove leading space and newline characters from the buffer.
void InputBuffer::skip_whitespace() {
    while (!_data.empty() && is_whitespace(_data.front())) {
        _data.erase(0, 1);
    }
}

bool InputBuffer::is_whitespace(const char character) {
    // Newline ends a token the same way space does (e.g. "Alice\n").
    return character == ' ' || character == '\n';
}

}
