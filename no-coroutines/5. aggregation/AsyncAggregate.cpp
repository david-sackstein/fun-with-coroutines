#include "common/event_loop/EventLoop.h"
#include "no-coroutines/3. async_tasks/AsyncTaskRunnerMarshalled.h"
#include "no-coroutines/5. aggregation/AsyncAggregate.h"

#include <functional>
#include <memory>

namespace no_coroutines {

void aggregate_totals(EventLoop &loop, std::shared_ptr<EventLoop::Work> *keepalive,
                      const std::function<void(int)> &on_complete) {
    const auto runner = std::make_shared<AsyncTaskRunnerMarshalled>(loop);
    const auto sum = std::make_shared<int>(0);

    runner->run_async_operation([runner, keepalive, sum, on_complete] {
        *sum += 10;
        runner->run_async_operation([runner, keepalive, sum, on_complete] {
            *sum += 20;
            runner->run_async_operation([keepalive, sum, on_complete] {
                *sum += 30;
                keepalive->reset();
                on_complete(*sum);
            });
        });
    });
}

int run_async_aggregate(EventLoop &loop) {
    int result = 0;
    auto keepalive = std::make_shared<EventLoop::Work>(loop);
    aggregate_totals(loop, &keepalive, [&result](const int sum) { result = sum; });
    loop.run();
    return result;
}

}
