#pragma once

#include <chrono>

namespace testing_delay {

#ifdef FWC_TESTING
inline constexpr auto async_task = std::chrono::milliseconds{10};
inline constexpr auto unmarshalled_wait = std::chrono::milliseconds{200};
inline constexpr auto reactor_safety = std::chrono::seconds{2};
#else
inline constexpr auto async_task = std::chrono::seconds{1};
inline constexpr auto unmarshalled_wait = std::chrono::seconds{5};
inline constexpr auto reactor_safety = std::chrono::seconds{10};
#endif

inline constexpr auto repl_safety = std::chrono::seconds{30};

}
