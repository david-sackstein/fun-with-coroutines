#pragma once

#include "common/generator/Tree.h"
#include "no-coroutines/01-generator/Fibonacci.h"
#include "no-coroutines/01-generator/TreeTraversal.h"

#include <gtest/gtest.h>
#include <vector>

class Demo01GeneratorNoCoroutines : public testing::Test {
protected:
    template<typename Generator>
    [[nodiscard]] static std::vector<int> drain(Generator &generator) {
        std::vector<int> values;
        for (const int value : generator) {
            values.push_back(value);
        }
        return values;
    }

    [[nodiscard]] static std::vector<int> collect_fibonacci(int count);
    [[nodiscard]] static std::vector<int> collect_preorder();
    [[nodiscard]] static std::vector<int> collect_inorder();
    [[nodiscard]] static bool fibonacci_second_pass_is_exhausted();
};
