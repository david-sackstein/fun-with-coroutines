#pragma once

// RAII guard that temporarily releases a unique_lock for its lifetime.
// The lock is re-acquired when the guard is destroyed.
// Used by EventLoop::invoke_task to drop the mutex while running a task.

#include <mutex>

class unlock_guard {
public:
    explicit unlock_guard(std::unique_lock<std::mutex>& lock)
        : lock_(lock)
    {
        lock_.unlock();
    }

    ~unlock_guard() {
        lock_.lock();
    }

    unlock_guard(const unlock_guard&) = delete;
    unlock_guard& operator=(const unlock_guard&) = delete;

private:
    std::unique_lock<std::mutex>& lock_;
};
