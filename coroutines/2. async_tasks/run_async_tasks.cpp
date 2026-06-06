#include "common/io/print.h"
#include "coroutines/2. async_tasks/AsyncTask.h"
#include "coroutines/3. async_tasks_marshalled/CoroutineObject.h"

#include <thread>

namespace coroutines {

using namespace std::chrono_literals;

static CoroutineObject my_task() {
    io::print("From thread {}\n", io::format_thread_id());

    io::print("Step 1\n");
    co_await AsyncTask{};

    io::print("From thread {}\n", io::format_thread_id());

    io::print("Step 2\n");
    co_await AsyncTask{};

    io::print("From thread {}\n", io::format_thread_id());

    io::print("Step 3\n");
}

void run_async_tasks() {
    [[maybe_unused]] auto t = my_task();
    
    // Sleep to keep main thread alive while async tasks complete
    std::this_thread::sleep_for(5s);
    
    io::print("Main thread done\n");
}

}
