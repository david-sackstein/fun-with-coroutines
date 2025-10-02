#pragma once

#include <mutex>
#include <cstdint>
#include "EventFd.h"
#include "Fd.h"

class NotifySignal {
public:
    NotifySignal() : _efd_raw(), _fd(_efd_raw.get()) {}

    // non-copyable
    NotifySignal(const NotifySignal&) = delete;
    NotifySignal& operator=(const NotifySignal&) = delete;

    Fd& arm() {
        std::lock_guard<std::mutex> lock(_mtx);
        if (_pending) {
            uint64_t v;
            _efd_raw.read(v);
            _pending = false;
        }
        return _fd;
    }

    void notify() {
        std::lock_guard<std::mutex> lock(_mtx);
        _efd_raw.write(1);
        _pending = true;
    }

private:
    EventFd _efd_raw;
    Fd _fd;
    std::mutex _mtx;
    bool _pending = false;
};
