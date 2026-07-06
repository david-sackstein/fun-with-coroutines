#pragma once

// Callback-based equivalent of the coroutines AsyncWork awaitable.
// run_async_operation() launches a background thread that runs the stored work,
// then calls the continuation directly on that same thread (unmarshalled).
// Compare with AsyncWorkMarshalled which posts the continuation to the EventLoop.

#include <functional>
#include <thread>

namespace no_coroutines {

class AsyncWork {
public:
    explicit AsyncWork(std::function<void()> work) : _work(std::move(work)) {}

    void run_async_operation(std::function<void()> continuation) const {
        std::thread t([work = _work, continuation] {
            work();
            continuation();
        });
        t.detach();
    }

private:
    std::function<void()> _work;
};

}
