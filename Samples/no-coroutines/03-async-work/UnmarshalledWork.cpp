#include "common/io/print.h"
#include "common/testing/Delays.h"
#include "no-coroutines/03-async-work/AsyncWork.h"
#include "no-coroutines/03-async-work/UnmarshalledWork.h"

#include <memory>
#include <thread>
#include <vector>

namespace no_coroutines {

void unmarshalled_work(std::shared_ptr<std::vector<std::string>> thread_ids);

// Launch the callback chain, wait for the background threads to finish, and return the thread IDs.
std::vector<std::string> collect_unmarshalled_thread_ids() {
    auto thread_ids = std::make_shared<std::vector<std::string>>();
    unmarshalled_work(thread_ids);

    // Sleep long enough for both background threads to complete their work.
    std::this_thread::sleep_for(testing_delay::unmarshalled_wait);
    return *thread_ids;
}

// Callback chain that records the current thread ID before and after each async step.
// Each step calls its continuation directly on the background thread that did the work.
void unmarshalled_work(std::shared_ptr<std::vector<std::string>> thread_ids) {
    auto runner = std::make_shared<AsyncWork>([] { std::this_thread::sleep_for(testing_delay::async_task); });

    // Record thread ID on the calling thread before the first async step.
    thread_ids->push_back(io::format_thread_id());

    runner->run_async_operation([runner, thread_ids] {
        // Running on a background thread. Record its thread ID after the first step.
        thread_ids->push_back(io::format_thread_id());

        runner->run_async_operation([thread_ids] {
            // Running on another background thread. Record its thread ID after the second step.
            thread_ids->push_back(io::format_thread_id());
        });
    });
}

}
