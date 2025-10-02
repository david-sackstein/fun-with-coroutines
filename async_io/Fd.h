#pragma once

#include <unistd.h>
#include <stdexcept>

class Fd {
    static constexpr int invalid_fd = -1;
    int _fd;

public:
    explicit Fd(int fd) : _fd(fd) {
        if (_fd <= invalid_fd) {
            throw std::runtime_error("Invalid file descriptor");
        }
    }

    ~Fd() { // non-moveable
        close(_fd);
    }

    // non-copyable,
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;

    int get() const noexcept { return _fd; }
};
