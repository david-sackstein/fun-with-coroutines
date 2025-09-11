#pragma once

#include <gtest/gtest.h>

#include <array>

class Demo05AggregationNoCoroutines : public testing::Test {
protected:
    [[nodiscard]] static int run_async_aggregate();
    [[nodiscard]] static std::array<int, 4> run_calculator_batch();
};
