#pragma once

// Wakeup mechanism for interrupting a blocking select() call.
//
// The Reactor includes the signal's fd in its read set each iteration.
// arm() clears any pending notification and returns the fd to watch.
// notify() fires the signal so select() returns immediately.

#include "EventFd.h"

#include <mutex>

class NotifySignal {
public:
    NotifySignal() = default;

    NotifySignal(const NotifySignal&) = delete;
    NotifySignal& operator=(const NotifySignal&) = delete;

    // Drain any pending notification and return the fd to include in select().
    int arm() {
        std::lock_guard lock(_mtx);
        if (_pending) {
            // A notification arrived before the last select(). Clear it now.
            _efd.read();
            _pending = false;
        }
        return _efd.get();
    }

    // Write a byte to the pipe so select() returns; no-op if already pending.
    void notify() {
        if (_pending) {
            return;
        }
        std::lock_guard lock(_mtx);
        _efd.write();
        _pending = true;
    }

private:
    EventFd _efd;
    std::mutex _mtx;
    bool _pending = false;
};
