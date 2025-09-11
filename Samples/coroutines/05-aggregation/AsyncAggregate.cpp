#include "common/event_loop/EventLoop.h"
#include "coroutines/03-async-tasks/AsyncTaskMarshalled.h"
#include "coroutines/05-aggregation/AsyncAggregate.h"
#include "coroutines/return_types/Task.h"

namespace coroutines {

// Each co_await suspends until the background thread completes and resumes on the EventLoop thread.
Task<int> aggregate_totals(EventLoop &loop) {
    // Keep the EventLoop alive until all three async steps are done.
    const EventLoop::Work guard(loop);

    int sum = 0;

    // Suspend until the first async step completes, then add its contribution.
    co_await AsyncTaskMarshalled{loop};
    sum += 10;

    // Suspend until the second async step completes, then add its contribution.
    co_await AsyncTaskMarshalled{loop};
    sum += 20;

    // Suspend until the third async step completes, then add its contribution.
    co_await AsyncTaskMarshalled{loop};
    sum += 30;

    // Return the accumulated total as the Task result.
    co_return sum;
}

}
