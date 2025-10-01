#pragma once

#include <unistd.h>
#include <stdexcept>

static const int invalid_fd = -1;

class Fd {
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
