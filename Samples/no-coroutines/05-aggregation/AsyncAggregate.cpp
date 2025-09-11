#include "common/event_loop/EventLoop.h"
#include "no-coroutines/03-async-tasks/AsyncTaskMarshalled.h"
#include "no-coroutines/05-aggregation/AsyncAggregate.h"

#include <functional>
#include <memory>

namespace no_coroutines {

// Nested callback chain that accumulates 10 + 20 + 30 across three async steps.
void aggregate_totals(EventLoop &loop, std::shared_ptr<EventLoop::Work> *keepalive,
                      const std::function<void(int)> &on_complete) {
    const auto runner = std::make_shared<AsyncTaskMarshalled>(loop);

    // sum is shared across all three callbacks via shared_ptr.
    const auto sum = std::make_shared<int>(0);

    runner->run_async_operation([runner, keepalive, sum, on_complete] {
        // First step complete. Add its contribution and start the second step.
        *sum += 10;
        runner->run_async_operation([runner, keepalive, sum, on_complete] {
            // Second step complete. Add its contribution and start the third step.
            *sum += 20;
            runner->run_async_operation([keepalive, sum, on_complete] {
                // Third step complete. Add its contribution and deliver the result.
                *sum += 30;

                // Release the keepalive so the EventLoop can stop.
                keepalive->reset();
                on_complete(*sum);
            });
        });
    });
}

// Wire up aggregate_totals, run the EventLoop, and return the final sum.
int run_async_aggregate(EventLoop &loop) {
    int result = 0;
    auto keepalive = std::make_shared<EventLoop::Work>(loop);
    aggregate_totals(loop, &keepalive, [&result](const int sum) { result = sum; });
    loop.run();
    return result;
}

}
