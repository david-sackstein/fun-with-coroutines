#include "common/event_loop/EventLoop.h"
#include "common/io/print.h"
#include "no-coroutines/3. async_tasks/AsyncTaskRunnerMarshalled.h"

#include <memory>

namespace no_coroutines {

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

void run_marshalled_sample() {
    io::print("\nAsync tasks — marshalled back to event loop (fixed):\n");
    auto keepalive = std::make_shared<EventLoop::Work>(g_loop);
    my_task_marshalled(g_loop, &keepalive);
    g_loop.run();
    io::print("Loop exited cleanly\n");
}

}
