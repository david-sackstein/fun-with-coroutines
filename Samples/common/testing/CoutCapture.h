#pragma once

// RAII guard that redirects std::cout to an internal buffer for the duration of its lifetime.
// Captures all output printed during reactor.run() so the test can assert on the result.

#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>

class CoutCapture {
public:
    CoutCapture() : _old_buf(std::cout.rdbuf(_buffer.rdbuf())) {}

    ~CoutCapture() {
        std::cout.rdbuf(_old_buf);
    }

    [[nodiscard]] std::string str() const {
        return _buffer.str();
    }

private:
    std::stringstream _buffer;
    std::streambuf *_old_buf;
};
