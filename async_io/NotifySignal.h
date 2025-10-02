#pragma once

#include <mutex>
#include <unistd.h>
#include <cerrno>
#include <system_error>

class NotifySignal {
public:
    NotifySignal() {
        if (::pipe(_pipe) < 0) {
            throw std::system_error(errno, std::generic_category(), "pipe() failed");
        }
    }

    ~NotifySignal() {
        ::close(_pipe[0]);
        ::close(_pipe[1]);
    }

    // non-copyable
    NotifySignal(const NotifySignal&) = delete;
    NotifySignal& operator=(const NotifySignal&) = delete;

    int arm() {
        std::lock_guard<std::mutex> lock(_mtx);
        if (_pending) {
            read_one();
            _pending = false;
        }
        return _pipe[0];
    }

    void notify() {
        std::lock_guard<std::mutex> lock(_mtx);
        write_one();
        _pending = true;
    }

private:
    void write_one() {
        char b = 1;
        ssize_t result;
        while ((result = ::write(_pipe[1], &b, 1)) < 0 && errno == EINTR) {
            // retry on EINTR
        }
        if (result < 0) {
            throw std::system_error(errno, std::generic_category(), "notify write failed");
        }
    }

    void read_one() {
        char b;
        ssize_t result;
        while ((result = ::read(_pipe[0], &b, 1)) < 0 && errno == EINTR) {
            // retry on EINTR
        }
        if (result < 0) {
            throw std::system_error(errno, std::generic_category(), "notify drain failed");
        }
    }

    int _pipe[2];
    std::mutex _mtx;
    bool _pending = false;
};
