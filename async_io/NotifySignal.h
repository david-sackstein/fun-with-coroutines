#pragma once

#include "Fd.h"
#include "PipeFds.h"

#include <cerrno>
#include <unistd.h>

class NotifySignal {
public:
    NotifySignal() :
        _pipe(),
        _read(_pipe.r),
        _write(_pipe.w) {}

    // non-copyable
    NotifySignal(const NotifySignal &) = delete;

    NotifySignal &operator=(const NotifySignal &) = delete;

    Fd &arm() {
        std::lock_guard<std::mutex> lock(_mtx);
        if (_hasNotified) {
            read_all();
            _hasNotified = false;
        }
        return _read;
    }

    void notify() {
        std::lock_guard<std::mutex> lock(_mtx);
        write_one();
        _hasNotified = false;
    }

private:
    void write_one() const noexcept {
        char b = 1;
        while (::write(_write.get(), &b, 1) < 0 && errno == EINTR) {
            // retry on EINTR
        }
    }

    void read_all() const noexcept {
        char b;
        while (::read(_read.get(), &b, 1) > 0) {
            // keep draining
        }
    }

    PipeFds _pipe;
    Fd _read;
    Fd _write;
    std::mutex _mtx;
    bool _hasNotified = false;
};


