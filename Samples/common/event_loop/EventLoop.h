#pragma once

// Single-threaded task queue that runs until all outstanding work is done.
//
// Tasks are submitted from any thread with post(). run() blocks the calling
// thread until the outstanding work count reaches zero. Work is counted
// separately from the task queue: a long-running async operation holds a Work
// object to keep run() from returning early.
//
// Example:
//   g_loop.post([] { io::print("hello\n"); });
//   g_loop.run();  ->  prints "hello" then returns

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>

class EventLoop {
public:
    // RAII guard that keeps the loop alive for the lifetime of the object.
    class Work {
    public:
        explicit Work(EventLoop &loop) : loop(loop) {
            loop.add_work();
        }
        ~Work() {
            loop.remove_work();
        }

    private:
        EventLoop &loop;
    };

    EventLoop();

    // Enqueue a task for execution on the loop thread; safe to call from any thread.
    void post(std::function<void()> task);
    // Decrement the outstanding work count; stop the loop if it reaches zero.
    void remove_work();
    // Block the calling thread, executing tasks until outstanding work reaches zero.
    void run();

private:
    // Increment the outstanding work count.
    void add_work();

    // Pop and execute the front task, releasing the lock during execution.
    void invoke_task(std::unique_lock<std::mutex> &lock);

    std::queue<std::function<void()>> tasks;
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::size_t outstanding_work;
};

extern EventLoop g_loop;
