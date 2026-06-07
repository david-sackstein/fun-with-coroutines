#include "common/event_loop/EventLoop.h"
#include "common/io/print.h"
#include "coroutines/3. async_tasks/AsyncTaskMarshalled.h"
#include "coroutines/3. async_tasks/MarshalledTask.h"
#include "coroutines/common/CoroutineObject.h"

#include <vector>

namespace coroutines {

namespace {

CoroutineObject marshalled_task(EventLoop &loop, std::vector<std::string> &thread_ids) {
    const EventLoop::Work guard(loop);

    thread_ids.push_back(io::format_thread_id());
    co_await AsyncTaskMarshalled{loop};
    thread_ids.push_back(io::format_thread_id());

    co_await AsyncTaskMarshalled{loop};
    thread_ids.push_back(io::format_thread_id());

    co_await AsyncTaskMarshalled{loop};
    thread_ids.push_back(io::format_thread_id());
}

}

std::vector<std::string> collect_marshalled_thread_ids(EventLoop &loop) {
    std::vector<std::string> thread_ids;
    [[maybe_unused]] auto task = marshalled_task(loop, thread_ids);
    loop.run();
    return thread_ids;
}

}
