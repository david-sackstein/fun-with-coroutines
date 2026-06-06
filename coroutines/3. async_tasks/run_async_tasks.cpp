#include "common/event_loop/EventLoop.h"
#include "common/io/print.h"
#include "coroutines/3. async_tasks/AsyncTask.h"
#include "coroutines/3. async_tasks/AsyncTaskMarshalled.h"
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

static CoroutineObject my_task_marshalled(EventLoop &loop) {
    const EventLoop::Work guard(loop);

    io::print("From thread {}\n", io::format_thread_id());

    io::print("Step 1\n");
    co_await AsyncTaskMarshalled{loop};

    io::print("From thread {}\n", io::format_thread_id());

    io::print("Step 2\n");
    co_await AsyncTaskMarshalled{loop};

    io::print("From thread {}\n", io::format_thread_id());

    io::print("Step 3\n");
}

void run_unmarshalled_demo() {
    io::print("\nAsync tasks — resume on worker thread (wrong):\n");
    [[maybe_unused]] auto t = my_task_unmarshalled();
    std::this_thread::sleep_for(5s);
    io::print("Main thread done\n");
}

void run_marshalled_demo() {
    io::print("\nAsync tasks — marshalled back to event loop (fixed):\n");
    [[maybe_unused]] auto t = my_task_marshalled(g_loop);
    g_loop.run();
    io::print("Loop exited cleanly\n");
}

void run_async_tasks() {
    run_unmarshalled_demo();
    run_marshalled_demo();
}

}
