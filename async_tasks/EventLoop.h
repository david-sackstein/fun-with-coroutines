#pragma once

#include <coroutine>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>

class EventLoop {
public:
    class Work {
    public:
        explicit Work(EventLoop &loop) : loop(loop) { loop.add_work(); }
        ~Work() { loop.remove_work(); }
        Work(const Work&) = delete;
        Work& operator=(const Work&) = delete;
    private:
        EventLoop &loop;
    };

    enum class StopMode { None, Graceful, Immediate };

    EventLoop();

    void run();

    void post(std::function<void()> task);
    void remove_work();

    void post_stop(StopMode stopMode);
    void stop_now();

private:
    void add_work();

    void invoke_task(std::unique_lock<std::mutex> &lock);
    void stop(StopMode);

    std::queue<std::function<void()>> tasks;
    mutable std::mutex mtx;
    std::condition_variable cv;
    StopMode stop_mode;
    std::size_t outstanding_work;
};

extern EventLoop g_loop;
