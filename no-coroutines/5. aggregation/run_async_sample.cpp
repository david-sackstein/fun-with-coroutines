#include "common/event_loop/EventLoop.h"
#include "common/io/print.h"
#include "no-coroutines/3. async_tasks/AsyncTaskRunnerMarshalled.h"

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

void run_async_sample() {
    io::print("\n=== Sample 2 — Marshalled async aggregate ===\n");

    auto keepalive = std::make_shared<EventLoop::Work>(g_loop);
    aggregate_totals(g_loop, &keepalive, [](const int sum) {
        io::print("sum: {}\n", sum);
    });
    g_loop.run();
}

}
