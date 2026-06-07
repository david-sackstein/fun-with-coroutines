#include "common/event_loop/EventLoop.h"
#include "common/io/print.h"
#include "coroutines/3. async_tasks/AsyncTaskMarshalled.h"
#include "coroutines/common/Task.h"

namespace coroutines {

Task<int> aggregate_totals(EventLoop &loop) {
    const EventLoop::Work guard(loop);

    int sum = 0;

    co_await AsyncTaskMarshalled{loop};
    sum += 10;

    co_await AsyncTaskMarshalled{loop};
    sum += 20;

    co_await AsyncTaskMarshalled{loop};
    sum += 30;

    co_return sum;
}

void run_async_sample() {
    io::print("\n=== Sample 2 — Marshalled async aggregate ===\n");

    Task<int> aggregate = aggregate_totals(g_loop);
    aggregate.start();
    g_loop.run();

    io::print("sum: {}\n", aggregate.get());
}

}
