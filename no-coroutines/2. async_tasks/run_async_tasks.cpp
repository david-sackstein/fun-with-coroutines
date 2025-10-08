#include "no-coroutines/2. async_tasks/AsyncTaskRunner.h"

#include <print>
#include <thread>

namespace no_coroutines {

using namespace std::chrono_literals;

static void my_task() {
    auto runner = std::make_shared<AsyncTaskRunner2>();

    std::print("From thread {}\n", std::this_thread::get_id());
    std::print("Step 1\n");

    // First async operation
    runner->run_async_operation([runner] {
        std::print("From thread {}\n", std::this_thread::get_id());
        std::print("Step 2\n");

        // Second async operation
        runner->run_async_operation([runner] {
            std::print("From thread {}\n", std::this_thread::get_id());
            std::print("Step 3\n");
        });
    });
}

void run_async_tasks() {
    my_task();
    
    // Sleep to keep main thread alive while async tasks complete
    std::this_thread::sleep_for(5s);
    
    std::print("Main thread done\n");
}

}
