#pragma once

#include "EventFd.h"

#include <mutex>

class NotifySignal {
public:
    NotifySignal() = default;

    // non-copyable
    NotifySignal(const NotifySignal&) = delete;
    NotifySignal& operator=(const NotifySignal&) = delete;

    int arm() {
        std::lock_guard<std::mutex> lock(_mtx);
        if (_pending) {
            _efd.read();
            _pending = false;
        }
        return _efd.get();
    }

    void notify() {
        if (_pending) {
            return;
        }
        std::lock_guard<std::mutex> lock(_mtx);
        _efd.write();
        _pending = true;
    }

private:
    EventFd _efd;
    std::mutex _mtx;
    bool _pending = false;
};
