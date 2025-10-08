#include "coroutines/async_tasks/CoroutineObject.h"
#include "coroutines/async_tasks_2/AsyncTask2.h"

#include <print>
#include <thread>

namespace coroutines {

using namespace std::chrono_literals;

CoroutineObject my_task_2() {
    std::print("From thread {}\n", std::this_thread::get_id());

    std::print("Step 1\n");
    co_await AsyncTask2{};

    std::print("From thread {}\n", std::this_thread::get_id());

    std::print("Step 2\n");
    co_await AsyncTask2{};

    std::print("From thread {}\n", std::this_thread::get_id());

    std::print("Step 3\n");
}

void run_async_tasks_2() {
    auto t = my_task_2();
    
    // Sleep to keep main thread alive while async tasks complete
    std::this_thread::sleep_for(5s);
    
    std::print("Main thread done\n");
}

}
