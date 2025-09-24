#include "EventLoop.h"
#include "CoroutineObject.h"
#include "AsyncTask.h"

#include <iostream>

using namespace std::chrono_literals;

CoroutineObject my_task(EventLoop &loop) {
    EventLoop::Work guard(loop);

    std::cout << "From thread " << std::this_thread::get_id() << std::endl;

    std::cout << "Step 1" << std::endl;
    co_await AsyncTask{loop};

    std::cout << "From thread " << std::this_thread::get_id() << std::endl;

    std::cout << "Step 2" << std::endl;
    co_await AsyncTask{loop};

    std::cout << "From thread " << std::this_thread::get_id() << std::endl;

    std::cout << "Step 3" << std::endl;
    co_return;
}

void run_async_tasks_in_event_loop() {

    auto t = my_task(g_loop);

    g_loop.run();

    std::cout << "Loop exited cleanly\n";
}
