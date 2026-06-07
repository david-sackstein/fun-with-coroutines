#include "common/io/print.h"
#include "no-coroutines/3. async_tasks/AsyncTaskRunner.h"

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

void run_unmarshalled_sample() {
    io::print("\nAsync tasks — resume on worker thread (wrong):\n");
    my_task_unmarshalled();
    std::this_thread::sleep_for(5s);
    io::print("Main thread done\n");
}

}
