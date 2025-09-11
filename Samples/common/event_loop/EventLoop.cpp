#include "EventLoop.h"
#include "unlock_guard.h"
#include "work_guard.h"

EventLoop g_loop{};

EventLoop::EventLoop() :
    outstanding_work(0) {}

// Enqueue the task and wake the loop thread.
void EventLoop::post(std::function<void()> task) {
    {
        std::lock_guard lg(mtx);
        tasks.push(std::move(task));
        ++outstanding_work;
    }
    cv.notify_one();
}

// Decrement outstanding work and wake the loop thread if it reaches zero.
void EventLoop::remove_work() {
    std::lock_guard lg(mtx);
    if (--outstanding_work == 0) {
        cv.notify_all();
    }
}

// Block until all work is done, executing tasks as they arrive.
void EventLoop::run() {
    std::unique_lock lock(mtx);

    while (true) {
        cv.wait(lock, [&] {
            return !tasks.empty() || outstanding_work == 0;
        });

        if (outstanding_work == 0) {
            // All work is accounted for. Stop the loop.
            break;
        }

        if (!tasks.empty()) {
            invoke_task(lock);
        }
    }
}

// Increment the outstanding work count.
void EventLoop::add_work() {
    std::lock_guard lg(mtx);
    ++outstanding_work;
}

// Pop the front task and run it with the lock released.
void EventLoop::invoke_task(std::unique_lock<std::mutex>& lock) {
    auto task = std::move(tasks.front());
    tasks.pop();

    unlock_guard unlock(lock);
    work_guard decrement(*this);

    task();
}
