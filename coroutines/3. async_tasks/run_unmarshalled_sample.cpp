#include "common/io/print.h"
#include "coroutines/3. async_tasks/AsyncTask.h"
#include "coroutines/common/CoroutineObject.h"

#include <thread>

namespace coroutines {

using namespace std::chrono_literals;

static CoroutineObject my_task_unmarshalled() {
    io::print("From thread {}\n", io::format_thread_id());

    io::print("Step 1\n");
    co_await AsyncTask{};

    io::print("From thread {}\n", io::format_thread_id());

    io::print("Step 2\n");
    co_await AsyncTask{};

    io::print("From thread {}\n", io::format_thread_id());

    io::print("Step 3\n");
}

void run_unmarshalled_sample() {
    io::print("\nAsync tasks — resume on worker thread (wrong):\n");
    [[maybe_unused]] auto t = my_task_unmarshalled();
    std::this_thread::sleep_for(5s);
    io::print("Main thread done\n");
}

}
