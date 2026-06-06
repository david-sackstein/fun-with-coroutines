#include "common/event_loop/EventLoop.h"
#include "common/io/print.h"
#include "no-coroutines/3. async_tasks/AsyncTaskRunner.h"
#include "no-coroutines/3. async_tasks/AsyncTaskRunnerMarshalled.h"

#include <memory>
#include <thread>

namespace no_coroutines {

using namespace std::chrono_literals;

static void my_task_unmarshalled() {
    auto runner = std::make_shared<AsyncTaskRunner2>();

    io::print("From thread {}\n", io::format_thread_id());
    io::print("Step 1\n");

    runner->run_async_operation([runner] {
        io::print("From thread {}\n", io::format_thread_id());
        io::print("Step 2\n");

        runner->run_async_operation([runner] {
            io::print("From thread {}\n", io::format_thread_id());
            io::print("Step 3\n");
        });
    });
}

static void my_task_marshalled(EventLoop &loop, std::shared_ptr<EventLoop::Work> *keepalive) {
    auto runner = std::make_shared<AsyncTaskRunnerMarshalled>(loop);

    io::print("From thread {}\n", io::format_thread_id());
    io::print("Step 1\n");

    runner->run_async_operation([runner, keepalive] {
        io::print("From thread {}\n", io::format_thread_id());
        io::print("Step 2\n");

        runner->run_async_operation([runner, keepalive] {
            io::print("From thread {}\n", io::format_thread_id());
            io::print("Step 3\n");
            keepalive->reset();
        });
    });
}

void run_unmarshalled_demo() {
    io::print("\nAsync tasks — resume on worker thread (wrong):\n");
    my_task_unmarshalled();
    std::this_thread::sleep_for(5s);
    io::print("Main thread done\n");
}

void run_marshalled_demo() {
    io::print("\nAsync tasks — marshalled back to event loop (fixed):\n");
    auto keepalive = std::make_shared<EventLoop::Work>(g_loop);
    my_task_marshalled(g_loop, &keepalive);
    g_loop.run();
    io::print("Loop exited cleanly\n");
}

void run_async_tasks() {
    run_unmarshalled_demo();
    run_marshalled_demo();
}

}
