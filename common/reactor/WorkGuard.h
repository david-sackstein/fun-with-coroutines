#pragma once

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
