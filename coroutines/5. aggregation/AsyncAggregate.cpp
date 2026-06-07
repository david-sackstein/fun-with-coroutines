#include "common/event_loop/EventLoop.h"
#include "coroutines/3. async_tasks/AsyncTaskMarshalled.h"
#include "coroutines/5. aggregation/AsyncAggregate.h"
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

}
