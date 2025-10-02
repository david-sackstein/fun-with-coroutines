#pragma once

#include "Fd.h"
#include "PipeFds.h"

#include <cerrno>
#include <unistd.h>
#include <atomic>

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
        if (_notified.exchange(false, std::memory_order_acq_rel)) {
            read_one();
        }
        return _read;
    }

    void notify() {
        write_one();
        _notified.store(true, std::memory_order_release);
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
    std::atomic<bool> _notified{false};
};


