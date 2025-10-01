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
    NotifySignal(const NotifySignal&) = delete;
    NotifySignal& operator=(const NotifySignal&) = delete;

    const Fd& read_end() const { return _read; }

    void notify() const noexcept {
        char b = 1;
        while (::write(_write.get(), &b, 1) < 0 && errno == EINTR) {
            // retry on EINTR
        }
    }

    void on_selected() const noexcept {
        char b;
        while (::read(_read.get(), &b, 1) < 0 && errno == EINTR) {
            // retry on EINTR
        }
    }

private:
    PipeFds _pipe;
    Fd _read;
    Fd _write;
};


