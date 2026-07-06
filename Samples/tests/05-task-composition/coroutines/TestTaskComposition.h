#pragma once

#include <gtest/gtest.h>

#include <vector>

class Demo05TaskCompositionCoroutines : public testing::Test {
protected:
    [[nodiscard]] static int run_task_chain();
    [[nodiscard]] static std::vector<int> run_calculator_batch();
};
