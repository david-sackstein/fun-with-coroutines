#include "coroutines/3. async_tasks_marshalled/CoroutineObject.h"
#include "coroutines/2. async_tasks/AsyncTask.h"

#include <print>
#include <thread>

namespace coroutines {

using namespace std::chrono_literals;

static CoroutineObject my_task() {
    std::print("From thread {}\n", std::this_thread::get_id());

    std::print("Step 1\n");
    co_await AsyncTask{};

    std::print("From thread {}\n", std::this_thread::get_id());

    std::print("Step 2\n");
    co_await AsyncTask{};

    std::print("From thread {}\n", std::this_thread::get_id());

    std::print("Step 3\n");
}

void run_async_tasks() {
    auto t = my_task();
    
    // Sleep to keep main thread alive while async tasks complete
    std::this_thread::sleep_for(5s);
    
    std::print("Main thread done\n");
}

}
