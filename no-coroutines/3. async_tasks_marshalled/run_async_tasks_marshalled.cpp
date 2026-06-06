#include "common/event_loop/EventLoop.h"
#include "no-coroutines/3. async_tasks_marshalled/AsyncTaskRunnerMarshalled.h"

#include <print>
#include <thread>
#include <memory>

namespace no_coroutines {

using namespace std::chrono_literals;

void my_task(EventLoop& loop, std::shared_ptr<EventLoop::Work>* keepalive) {
    auto runner = std::make_shared<AsyncTaskRunnerMarshalled>(loop);

    std::print("From thread {}\n", std::this_thread::get_id());
    std::print("Step 1\n");

    // First async operation
    runner->run_async_operation([runner, keepalive] {
        std::print("From thread {}\n", std::this_thread::get_id());
        std::print("Step 2\n");

        // Second async operation
        runner->run_async_operation([runner, keepalive] {
            std::print("From thread {}\n", std::this_thread::get_id());
            std::print("Step 3\n");
            keepalive->reset();
        });
    });
}

void run_async_tasks_marshalled() {
    auto keepalive = std::make_shared<EventLoop::Work>(g_loop);
    my_task(g_loop, &keepalive);

    g_loop.run();

    std::print("Loop exited cleanly\n");
}

}
