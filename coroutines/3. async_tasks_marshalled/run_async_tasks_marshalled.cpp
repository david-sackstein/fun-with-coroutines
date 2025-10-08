#include "coroutines/3. async_tasks_marshalled/CoroutineObject.h"
#include "coroutines/3. async_tasks_marshalled/AsyncTaskMarshalled.h"

#include "common/event_loop/EventLoop.h"

#include <print>

namespace coroutines {

using namespace std::chrono_literals;

CoroutineObject my_task(EventLoop &loop) {
    EventLoop::Work guard(loop);

    std::print("From thread {}\n", std::this_thread::get_id());

    std::print("Step 1\n");
    co_await AsyncTaskMarshalled{loop};

    std::print("From thread {}\n", std::this_thread::get_id());

    std::print("Step 2\n");
    co_await AsyncTaskMarshalled{loop};

    std::print("From thread {}\n", std::this_thread::get_id());

    std::print("Step 3\n");
}

void run_async_tasks_marshalled() {

    auto t = my_task(g_loop);

    g_loop.run();

    std::print("Loop exited cleanly\n");
}

}
