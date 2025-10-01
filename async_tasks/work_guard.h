#pragma once

#include <mutex>
#include <condition_variable>

struct work_guard {
    std::size_t& outstanding_work;
    std::condition_variable& cv;
    std::mutex& mtx;

    explicit work_guard(std::size_t& ow, std::condition_variable& cv_, std::mutex& mtx_)
        : outstanding_work(ow), cv(cv_), mtx(mtx_) {}

    ~work_guard() {
        std::lock_guard<std::mutex> lock(mtx);
        if (--outstanding_work == 0) {
            cv.notify_all();
        }
    }

    // non-copyable
    work_guard(const work_guard&) = delete;
    work_guard& operator=(const work_guard&) = delete;
};