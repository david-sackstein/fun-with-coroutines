#pragma once

// RAII guard that calls EventLoop::remove_work() on destruction.
// Used by EventLoop::invoke_task to decrement the work count after a task runs.

#include "EventLoop.h"

struct work_guard {
    EventLoop& loop;

    explicit work_guard(EventLoop& l) : loop(l) {}

    ~work_guard() {
        loop.remove_work();
    }
};