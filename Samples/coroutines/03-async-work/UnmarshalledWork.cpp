#include "common/io/print.h"
#include "common/testing/Delays.h"
#include "coroutines/03-async-work/AsyncWork.h"
#include "coroutines/03-async-work/UnmarshalledWork.h"
#include "coroutines/return_types/NeverSuspendCoroutine.h"

#include <thread>
#include <vector>

namespace coroutines {

NeverSuspendCoroutine unmarshalled_work(std::vector<std::string> &thread_ids);

// Launch the coroutine, wait for the background threads to finish, and return the thread IDs.
std::vector<std::string> collect_unmarshalled_thread_ids() {
    std::vector<std::string> thread_ids;
    [[maybe_unused]] auto task = unmarshalled_work(thread_ids);

    // Sleep long enough for both background threads to complete their work.
    std::this_thread::sleep_for(testing_delay::unmarshalled_wait);
    return thread_ids;
}

// Coroutine that records the current thread ID before and after each async step.
// Each co_await resumes directly on the background thread that did the work.
NeverSuspendCoroutine unmarshalled_work(std::vector<std::string> &thread_ids) {
    // Record thread ID on the original (calling) thread before the first step.
    thread_ids.push_back(io::format_thread_id());

    co_await AsyncWork{[] {
        std::this_thread::sleep_for(testing_delay::async_task);
    }};

    // Resumed on a background thread. Record its thread ID.
    thread_ids.push_back(io::format_thread_id());

    co_await AsyncWork{[] {
        std::this_thread::sleep_for(testing_delay::async_task);
    }};

    // Resumed on another background thread. Record its thread ID.
    thread_ids.push_back(io::format_thread_id());
}

}
