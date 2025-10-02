#pragma once

#include <unistd.h>
#include <cerrno>
#include <system_error>

// emulate eventfd for compatibility with non-linux OS
class EventFd {
public:
    EventFd(unsigned int /*init*/ = 0, int /*flags*/ = 0) {
        if (::pipe(_pipe) < 0)
            throw std::system_error(errno, std::generic_category());
    }

    ~EventFd() {
        ::close(_pipe[0]);
        ::close(_pipe[1]);
    }

    int get() const { return _pipe[0]; } // read end

    void write(uint64_t /*v*/) {
        char b = 1;
        ::write(_pipe[1], &b, 1);
    }

    void read(uint64_t &v) {
        char b;
        ::read(_pipe[0], &b, 1);
        v = 1;
    }

private:
    int _pipe[2];
};
