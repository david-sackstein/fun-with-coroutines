#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
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

private:
    void add_work();

    void invoke_task(std::unique_lock<std::mutex> &lock);

    std::queue<std::function<void()>> tasks;
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::size_t outstanding_work;
};

extern EventLoop g_loop;
