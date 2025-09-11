#pragma once

#include <mutex>

class unlock_guard {
    std::unique_lock<std::mutex>& lock_;
public:
    explicit unlock_guard(std::unique_lock<std::mutex>& lock)
        : lock_(lock)
    {
        lock_.unlock();
    }

    ~unlock_guard() {
        lock_.lock();    // re-lock when destroyed
    }

    // non-copyable
    unlock_guard(const unlock_guard&) = delete;
    unlock_guard& operator=(const unlock_guard&) = delete;
};