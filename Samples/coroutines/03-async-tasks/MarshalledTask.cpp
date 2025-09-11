#include "common/event_loop/EventLoop.h"
#include "common/io/print.h"
#include "common/testing/Delays.h"
#include "coroutines/03-async-tasks/AsyncTaskMarshalled.h"
#include "coroutines/03-async-tasks/MarshalledTask.h"
#include "coroutines/return_types/NeverSuspendCoroutine.h"

#include <thread>
#include <vector>

namespace coroutines {

NeverSuspendCoroutine marshalled_task(EventLoop &loop, std::vector<std::string> &thread_ids);

// Launch the coroutine and run the loop until all steps are done; return the recorded thread IDs.
std::vector<std::string> collect_marshalled_thread_ids(EventLoop &loop) {
    std::vector<std::string> thread_ids;
    [[maybe_unused]] auto task = marshalled_task(loop, thread_ids);
    loop.run();
    return thread_ids;
}

// Coroutine that records the current thread ID before and after each async step.
// Each co_await suspends, performs work on a background thread, and resumes on the EventLoop thread.
NeverSuspendCoroutine marshalled_task(EventLoop &loop, std::vector<std::string> &thread_ids) {
    // Keep the EventLoop alive until the coroutine completes.
    const EventLoop::Work guard(loop);

    // Record thread ID before the first async step.
    thread_ids.push_back(io::format_thread_id());
    co_await AsyncTaskMarshalled{loop, [] { std::this_thread::sleep_for(testing_delay::async_task); }};

    // Resumed on the EventLoop thread. Record thread ID after the first step.
    thread_ids.push_back(io::format_thread_id());

    co_await AsyncTaskMarshalled{loop, [] { std::this_thread::sleep_for(testing_delay::async_task); }};

    // Resumed on the EventLoop thread. Record thread ID after the second step.
    thread_ids.push_back(io::format_thread_id());

    co_await AsyncTaskMarshalled{loop, [] { std::this_thread::sleep_for(testing_delay::async_task); }};

    // Resumed on the EventLoop thread. Record thread ID after the third step.
    thread_ids.push_back(io::format_thread_id());
}

}
