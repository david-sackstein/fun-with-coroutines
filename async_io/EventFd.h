#pragma once

#include <unistd.h>
#include <cerrno>
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

    int get() const { return _pipe[0]; } // read descriptor for select/poll

    void write() {
        char b = 1;
        // retry on EINTR
        while (::write(_pipe[1], &b, 1) < 0 && errno == EINTR) {}

        if (errno != 0) {
            throw std::system_error(errno, std::generic_category(), "EventFd notify failed");
        }
    }

    // clear the event (read one byte internally)
    void read() {
        char b;
        // retry on EINTR
        while (::read(_pipe[0], &b, 1) < 0 && errno == EINTR) {}

        if (errno != 0) {
            throw std::system_error(errno, std::generic_category(), "EventFd clear failed");
        }
    }

private:
    int _pipe[2];
};
