#pragma once

// RAII guard that spawns a safety thread alongside a Reactor::run() call.
// If the Reactor is still running when the timeout expires the thread stops it.
// On destruction the guard signals the thread to wake early and joins it,
// ensuring no dangling references survive after the guard goes out of scope.

#include "common/reactor/Reactor.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

class ReactorStopper {
public:
    ReactorStopper(Reactor &reactor, std::chrono::nanoseconds timeout)
        : _reactor(reactor) {
        _thread = std::thread([this, timeout] {
            std::unique_lock lock(_mtx);
            if (!_cv.wait_for(lock, timeout, [this] { return _done; })) {
                _reactor.stop();
            }
        });
    }

    ~ReactorStopper() noexcept {
        {
            std::lock_guard lock(_mtx);
            _done = true;
        }
        _cv.notify_one();
        _thread.join();
    }

    ReactorStopper(const ReactorStopper &) = delete;
    ReactorStopper &operator=(const ReactorStopper &) = delete;
    ReactorStopper(ReactorStopper &&) = delete;
    ReactorStopper &operator=(ReactorStopper &&) = delete;

private:
    Reactor &_reactor;
    std::mutex _mtx;
    std::condition_variable _cv;
    bool _done = false;
    std::thread _thread;
};
