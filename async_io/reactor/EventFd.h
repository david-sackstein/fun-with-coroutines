#pragma once

#include <unistd.h>
#include <system_error>

class EventFd {
public:
    EventFd() {
        if (::pipe(_pipe) < 0) {
            throw std::system_error(errno, std::generic_category());
        }
    }

    ~EventFd() {
        ::close(_pipe[0]);
        ::close(_pipe[1]);
    }

    // non-copyable, non-movable
    EventFd(const EventFd&) = delete;
    EventFd& operator=(const EventFd&) = delete;

    int get() const { return _pipe[0]; } // read descriptor for select/poll

    void write() {
        char b = 1;
        ssize_t n;
        // retry on EINTR
        while ((n = ::write(_pipe[1], &b, 1)) < 0 && errno == EINTR) {}
        if (n < 0) {
            throw std::system_error(errno, std::generic_category(), "EventFd write failed");
        }
    }

    void read() {
        char b;
        ssize_t n;
        // retry on EINTR
        while ((n = ::read(_pipe[0], &b, 1)) < 0 && errno == EINTR) {}
        if (n < 0) {
            throw std::system_error(errno, std::generic_category(), "EventFd read failed");
        }
    }

private:
    int _pipe[2];
};
