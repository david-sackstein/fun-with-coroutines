#pragma once

#include <gtest/gtest.h>
#include <string>
#include <string_view>

class Demo04CalculatorNoCoroutines : public testing::Test {
protected:
    [[nodiscard]] static std::string run_calculator_session(std::string_view input_line);
};
