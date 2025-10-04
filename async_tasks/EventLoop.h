#pragma once

#include <coroutine>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>

class EventLoop {
public:
    class Work {
    public:
        explicit Work(EventLoop &loop) : loop(loop) { loop.add_work(); }
        ~Work() { loop.remove_work(); }

    private:
        EventLoop &loop;
    };

    EventLoop();

    void run();

    void post(std::function<void()> task);
    void remove_work();
    std::size_t get_outstanding_work() const { return outstanding_work; }

private:
    void add_work();

    void invoke_task(std::unique_lock<std::mutex> &lock);

    std::queue<std::function<void()>> tasks;
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::size_t outstanding_work;
};

extern EventLoop g_loop;
