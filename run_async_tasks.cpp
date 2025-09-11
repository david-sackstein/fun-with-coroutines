#include "async_tasks/event_loop/EventLoop.h"
#include "async_tasks/CoroutineObject.h"
#include "async_tasks/AsyncTask.h"

#include <print>

using namespace std::chrono_literals;

CoroutineObject my_task(EventLoop &loop) {
    EventLoop::Work guard(loop);

    std::print("From thread {}\n", std::this_thread::get_id());

    std::print("Step 1\n");
    co_await AsyncTask{loop};

    std::print("From thread {}\n", std::this_thread::get_id());

    std::print("Step 2\n");
    co_await AsyncTask{loop};

    std::print("From thread {}\n", std::this_thread::get_id());

    std::print("Step 3\n");
}

void run_async_tasks_in_event_loop() {

    auto t = my_task(g_loop);

    g_loop.run();

    std::print("Loop exited cleanly\n");
}
