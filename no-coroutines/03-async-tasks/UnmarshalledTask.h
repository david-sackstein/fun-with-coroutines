#pragma once

#include <string>
#include <vector>

namespace no_coroutines {

[[nodiscard]] std::vector<std::string> collect_unmarshalled_thread_ids();

}
