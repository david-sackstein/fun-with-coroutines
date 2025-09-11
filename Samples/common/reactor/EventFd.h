#pragma once

// Self-pipe used as a wakeup event source for select().
// write() signals an event by writing one byte to the write end.
// read() drains that byte. get() returns the read end for use in select().

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

    EventFd(const EventFd&) = delete;
    EventFd& operator=(const EventFd&) = delete;

    // Return the read end of the pipe; pass this to select() to watch for signals.
    [[nodiscard]] int get() const {
        return _pipe[0];
    }

    // Write one byte to the write end to signal an event; retries on EINTR.
    void write() const {
        char b = 1;
        ssize_t n;
        while ((n = ::write(_pipe[1], &b, 1)) < 0 && errno == EINTR) {}
        if (n < 0) {
            throw std::system_error(errno, std::generic_category(), "EventFd write failed");
        }
    }

    // Drain one byte from the read end; retries on EINTR.
    void read() const {
        char b;
        ssize_t n;
        while ((n = ::read(_pipe[0], &b, 1)) < 0 && errno == EINTR) {}
        if (n < 0) {
            throw std::system_error(errno, std::generic_category(), "EventFd read failed");
        }
    }

private:
    int _pipe[2]{};
};
