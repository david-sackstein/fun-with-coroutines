#pragma once

#include <unistd.h>
#include <cstdint>
#include <cerrno>
#include <fcntl.h>
#include <system_error>

// Linux version: uses actual eventfd
#if defined(__linux__)
#include <sys/eventfd.h>
class EventFd {
public:
    EventFd(unsigned int init = 0, int flags = 0) {
        _fd = eventfd(init, flags);
        if (_fd < 0) throw std::system_error(errno, std::generic_category());
    }

    ~EventFd() { if (_fd >= 0) ::close(_fd); }

    int get() const { return _fd; }

    void write(uint64_t v) { ::write(_fd, &v, sizeof(v)); }

    void read(uint64_t &v) { ::read(_fd, &v, sizeof(v)); }

private:
    int _fd;
};

#else // macOS / fallback
class EventFd {
public:
    EventFd(unsigned int /*init*/ = 0, int /*flags*/ = 0) {
        if (::pipe(_pipe) < 0)
            throw std::system_error(errno, std::generic_category());

        // optional: make read/write non-blocking
        // fcntl(_pipe[0], F_SETFL, O_NONBLOCK);
        // fcntl(_pipe[1], F_SETFL, O_NONBLOCK);
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
#endif
