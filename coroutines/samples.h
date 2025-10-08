#pragma once

namespace coroutines {
    void run_generator();
    void run_async_tasks();
    void run_async_tasks_marshalled();
    void run_async_io();
}

using namespace coroutines;