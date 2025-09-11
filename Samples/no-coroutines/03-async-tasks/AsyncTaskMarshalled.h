#pragma once

// Callback-based equivalent of the coroutines AsyncTaskMarshalled awaitable.
// run_async_operation() launches a background thread that runs the stored work,
// then posts the continuation back to the EventLoop. The continuation therefore
// runs on the EventLoop thread, not on the background thread.
// Compare with the coroutines version where this marshalling is expressed as co_await.

#include "common/event_loop/EventLoop.h"

#include <functional>
#include <thread>

namespace no_coroutines {

class AsyncTaskMarshalled {
public:
    AsyncTaskMarshalled(EventLoop &loop, std::function<void()> work)
        : _loop(loop), _work(std::move(work)) {}

    void run_async_operation(std::function<void()> continuation) const {
        std::thread t([work = _work, continuation, &loop = _loop] {
            work();
            loop.post(continuation);
        });
        t.detach();
    }

private:
    EventLoop &_loop;
    std::function<void()> _work;
};

}
