#pragma once

#include "EventLoop.h"

struct work_guard {
    EventLoop& loop;

    explicit work_guard(EventLoop& l) : loop(l) {}

    ~work_guard() {
        loop.remove_work();
    }
};