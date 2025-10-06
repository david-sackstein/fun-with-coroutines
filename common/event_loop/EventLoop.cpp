#include "EventLoop.h"
#include "unlock_guard.h"
#include "work_guard.h"

EventLoop g_loop{};

EventLoop::EventLoop() :
    outstanding_work(0) {}

void EventLoop::post(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lg(mtx);
        tasks.push(std::move(task));
        ++outstanding_work;
    }
    cv.notify_one();
}

void EventLoop::add_work() {
    std::lock_guard<std::mutex> lg(mtx);
    ++outstanding_work;
}

void EventLoop::remove_work() {
    std::lock_guard<std::mutex> lg(mtx);
    if (--outstanding_work == 0) {
        cv.notify_all();
    }
}

void EventLoop::run() {
    std::unique_lock<std::mutex> lock(mtx);

    while (true) {
        cv.wait(lock, [&] {
            return ! tasks.empty() || outstanding_work == 0;
        });

        if (outstanding_work == 0) {
            break;
        }

        if (!tasks.empty()) {
            invoke_task(lock);
        }
    }
}

// assumes we are under the lock
void EventLoop::invoke_task(std::unique_lock<std::mutex>& lock) {
    auto task = std::move(tasks.front());
    tasks.pop();

    unlock_guard unlock(lock);
    work_guard decrement(*this);

    task();
}
