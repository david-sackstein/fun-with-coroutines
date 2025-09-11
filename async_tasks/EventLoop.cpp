#include "EventLoop.h"

EventLoop g_loop{};

EventLoop::EventLoop() :
    stop_mode(StopMode::None),
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
            return stop_mode != StopMode::None || ! tasks.empty() || outstanding_work > 1;
        });

        if (stop_mode == StopMode::Immediate) {
            break;
        }

        if (stop_mode == StopMode::Graceful && outstanding_work == 0) {
            break;
        }

        if (!tasks.empty()) {
            invoke_task(lock);
        }
    }
}

void EventLoop::stop_now() {
    stop(StopMode::Immediate);
}

void EventLoop::post_stop(StopMode stopMode) {
    post([=, this]{
        stop(stopMode);
        remove_work();
    });
}

void EventLoop::invoke_task(std::unique_lock<std::mutex> &lock) {
    auto task = std::move(tasks.front());
    tasks.pop();

    lock.unlock();
    try {
        task();
    }
    catch(...) {
        lock.lock();
        --outstanding_work;
        throw;
    }
    lock.lock();

    if (--outstanding_work == 0) {
        cv.notify_all();
    }
}

void EventLoop::stop(StopMode mode) {
    {
        std::lock_guard<std::mutex> lg(mtx);
        stop_mode = mode;
    }
    cv.notify_all();
}

