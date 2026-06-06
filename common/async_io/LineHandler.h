#pragma once

#include "common/async_io/CalcLine.h"

#include <functional>
#include <string>
#include <string_view>

namespace async_io {

using LineHandler = std::function<std::string(std::string_view request)>;

inline LineHandler echo_line_handler() {
    return [](const std::string_view request) {
        return std::string(request);
    };
}

inline LineHandler calc_line_handler() {
    return [](const std::string_view request) {
        return eval_binary_expression(request);
    };
}

[[nodiscard]] inline bool response_matches(const std::string_view request, const std::string_view received,
                                           const LineHandler &handler) {
    return received == handler(request);
}

}
