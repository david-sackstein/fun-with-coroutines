#include "common/event_loop/EventLoop.h"
#include "common/io/print.h"
#include "common/testing/Delays.h"
#include "no-coroutines/03-async-tasks/AsyncTaskMarshalled.h"
#include "no-coroutines/03-async-tasks/MarshalledTask.h"

#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace no_coroutines {

void marshalled_task(EventLoop &loop, std::shared_ptr<EventLoop::Work> *keepalive,
                     std::shared_ptr<std::vector<std::string>> thread_ids);

// Launch the callback chain and run the loop until all steps are done; return the thread IDs.
std::vector<std::string> collect_marshalled_thread_ids(EventLoop &loop) {
    auto thread_ids = std::make_shared<std::vector<std::string>>();
    auto keepalive = std::make_shared<EventLoop::Work>(loop);
    marshalled_task(loop, &keepalive, thread_ids);
    loop.run();
    return *thread_ids;
}

// Callback chain that records the current thread ID before and after each async step.
// Each step posts its continuation to the EventLoop, so all steps run on the loop thread.
void marshalled_task(EventLoop &loop, std::shared_ptr<EventLoop::Work> *keepalive,
                     std::shared_ptr<std::vector<std::string>> thread_ids) {
    const auto runner = std::make_shared<AsyncTaskMarshalled>(loop, [] { std::this_thread::sleep_for(testing_delay::async_task); });

    // Record thread ID on the calling thread before the first async step.
    thread_ids->push_back(io::format_thread_id());

    runner->run_async_operation([runner, keepalive, thread_ids] {
        // Resumed on the EventLoop thread. Record thread ID after the first step.
        thread_ids->push_back(io::format_thread_id());

        runner->run_async_operation([runner, keepalive, thread_ids] {
            // Resumed on the EventLoop thread. Record thread ID after the second step.
            thread_ids->push_back(io::format_thread_id());

            runner->run_async_operation([keepalive, thread_ids] {
                // Resumed on the EventLoop thread. Record thread ID after the third step.
                thread_ids->push_back(io::format_thread_id());

                // Release the keepalive so the EventLoop can stop.
                keepalive->reset();
            });
        });
    });
}

}
