#pragma once

// GCC 13: std::format (C++20) yes, <print>/std::print (C++23) no.
// GCC 13: std::formatter<std::thread::id> no — use format_thread_id().

#include <format>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

namespace io {

template<typename... Args>
void print(std::format_string<Args...> fmt, Args&&... args) {
    std::cout << std::format(fmt, std::forward<Args>(args)...);
}

[[nodiscard]] inline std::string format_thread_id() {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}

}