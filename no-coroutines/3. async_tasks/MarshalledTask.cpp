#include "common/event_loop/EventLoop.h"
#include "common/io/print.h"
#include "no-coroutines/3. async_tasks/AsyncTaskRunnerMarshalled.h"
#include "no-coroutines/3. async_tasks/MarshalledTask.h"

#include <functional>
#include <memory>
#include <vector>

namespace no_coroutines {

namespace {

void marshalled_task(EventLoop &loop, std::shared_ptr<EventLoop::Work> *keepalive,
                     std::shared_ptr<std::vector<std::string>> thread_ids) {
    const auto runner = std::make_shared<AsyncTaskRunnerMarshalled>(loop);

    thread_ids->push_back(io::format_thread_id());

    runner->run_async_operation([runner, keepalive, thread_ids] {
        thread_ids->push_back(io::format_thread_id());

        runner->run_async_operation([runner, keepalive, thread_ids] {
            thread_ids->push_back(io::format_thread_id());

            runner->run_async_operation([keepalive, thread_ids] {
                thread_ids->push_back(io::format_thread_id());
                keepalive->reset();
            });
        });
    });
}

}

std::vector<std::string> collect_marshalled_thread_ids(EventLoop &loop) {
    auto thread_ids = std::make_shared<std::vector<std::string>>();
    auto keepalive = std::make_shared<EventLoop::Work>(loop);
    marshalled_task(loop, &keepalive, thread_ids);
    loop.run();
    return *thread_ids;
}

}
