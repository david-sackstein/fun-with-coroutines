#include "common/event_loop/EventLoop.h"
#include "common/io/print.h"
#include "coroutines/3. async_tasks/AsyncTaskMarshalled.h"
#include "coroutines/common/CoroutineObject.h"

namespace coroutines {

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

void run_marshalled_sample() {
    io::print("\nAsync tasks — marshalled back to event loop (fixed):\n");
    [[maybe_unused]] auto t = my_task_marshalled(g_loop);
    g_loop.run();
    io::print("Loop exited cleanly\n");
}

}
