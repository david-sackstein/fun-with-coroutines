#pragma once

#include <functional>
#include <memory>

class EventLoop;

namespace no_coroutines {

void aggregate_totals(EventLoop &loop, std::shared_ptr<EventLoop::Work> *keepalive,
                      const std::function<void(int)> &on_complete);

[[nodiscard]] int run_async_aggregate(EventLoop &loop);

}
