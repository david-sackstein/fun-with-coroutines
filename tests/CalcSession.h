#pragma once

#include "tests/Side.h"

#include <string>

namespace test_helpers {

[[nodiscard]] std::string run_calculator_session(Side side, std::string_view input_line);

}
