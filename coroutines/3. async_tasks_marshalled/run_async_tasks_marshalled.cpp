#include "common/event_loop/EventLoop.h"
#include "common/io/print.h"
#include "coroutines/3. async_tasks_marshalled/AsyncTaskMarshalled.h"
#include "coroutines/3. async_tasks_marshalled/CoroutineObject.h"

namespace coroutines {

using namespace std::chrono_literals;

CoroutineObject my_task(EventLoop &loop) {
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

void run_async_tasks_marshalled() {

    [[maybe_unused]] auto t = my_task(g_loop);

    g_loop.run();

    io::print("Loop exited cleanly\n");
}

}
