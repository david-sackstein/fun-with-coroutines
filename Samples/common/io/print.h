#pragma once

// Formatting and printing utilities.
//
// GCC 13 supports std::format (C++20) but not std::print (C++23), so io::print
// writes to std::cout via std::format. std::formatter<std::thread::id> is also
// absent in GCC 13, so format_thread_id() converts a thread ID to string manually.

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