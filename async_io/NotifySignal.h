#pragma once

#include "Fd.h"
#include "PipeFds.h"

#include <cerrno>
#include <unistd.h>
#include <mutex>

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
        if (_notified) {
            read_one();
            _notified = false;
        }
        return _read;
    }

    void notify() {
        std::lock_guard<std::mutex> lock(_mtx);
        write_one();
        _notified = true;
    }

private:
    void write_one() const noexcept {
        char b = 1;
        while (::write(_write.get(), &b, 1) < 0 && errno == EINTR) {
            // retry on EINTR
        }
    }

    void read_one() const noexcept {
        char b;
        while (::read(_read.get(), &b, 1) < 0 && errno == EINTR) {
            // retry on EINTR
        }
    }

    PipeFds _pipe;
    Fd _read;
    Fd _write;
    std::mutex _mtx;
    bool _notified = false;
};


