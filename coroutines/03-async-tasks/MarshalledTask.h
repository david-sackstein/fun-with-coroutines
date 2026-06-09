#pragma once

#include <string>
#include <vector>

class EventLoop;

namespace coroutines {

[[nodiscard]] std::vector<std::string> collect_marshalled_thread_ids(EventLoop &loop);

}
