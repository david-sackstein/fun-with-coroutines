#include "EventLoop.h"
#include "CoroutineObject.h"
#include "AsyncTask.h"

#include <iostream>
#include <memory>
#include <csignal>
#include <unistd.h>
#include <cstring>

using namespace std::chrono_literals;

CoroutineObject my_task(EventLoop &loop) {
    std::cout << "Step 1\n";
    co_await AsyncTask{loop};

    std::cout << "Step 2\n";
    co_await AsyncTask{loop};

    std::cout << "Step 3\n";

    loop.post_stop(EventLoop::StopMode::Immediate);
}

void handle_sigint(int) {
    // Use signal-safe write() instead of std::cout
    const char* msg = "Caught SIGINT\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    g_loop.stop_now();
}

void run_async_tasks_in_event_loop() {
    // Set signal handler
    std::signal(SIGINT, handle_sigint);

    // Work guard keeps loop alive
    EventLoop::Work guard(g_loop);

    auto t = my_task(g_loop);

    g_loop.run();

    std::cout << "Loop exited cleanly\n";
}
