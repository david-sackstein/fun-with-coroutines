#pragma once

// RAII guard that keeps the Reactor running for the lifetime of the object.
// Increments the Reactor's work count on construction and decrements it on destruction.
// When the last WorkGuard is destroyed the Reactor stops.

#include "Reactor.h"

class WorkGuard {
public:
    explicit WorkGuard(Reactor& reactor) : _reactor(reactor) {
        _reactor.add_work();
    }

    ~WorkGuard() noexcept {
        _reactor.remove_work();
    }

private:
    Reactor& _reactor;
};
