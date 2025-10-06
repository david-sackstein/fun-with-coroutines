#include "common/event_loop/EventLoop.h"
#include "no-coroutines/async_tasks/AsyncTaskRunner.h"

#include <print>
#include <thread>
#include <memory>

namespace no_coroutines {

using namespace std::chrono_literals;

void my_task(EventLoop& loop) {
    // Create a shared runner to manage the work guard lifetime
    auto runner = std::make_shared<AsyncTaskRunner>(loop);

    std::print("From thread {}\n", std::this_thread::get_id());
    std::print("Step 1\n");

    // First async operation
    runner->run_async_operation([runner, &loop] {
        std::print("From thread {}\n", std::this_thread::get_id());
        std::print("Step 2\n");

        // Second async operation
        runner->run_async_operation([runner, &loop] {
            std::print("From thread {}\n", std::this_thread::get_id());
            std::print("Step 3\n");
        });
    });
}

void run_async_tasks_over_event_loop() {
    my_task(g_loop);
    
    g_loop.run();
    
    std::print("Loop exited cleanly\n");
}

}
