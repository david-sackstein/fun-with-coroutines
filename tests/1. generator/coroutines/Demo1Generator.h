#pragma once

#include "common/generator/Tree.h"
#include "coroutines/1. generator/Fibonacci.h"
#include "coroutines/1. generator/TreeTraversal.h"

#include <gtest/gtest.h>

#include <vector>

class Demo1GeneratorCoroutines : public testing::Test {
protected:
    template<typename Generator>
    [[nodiscard]] static std::vector<int> drain(Generator &generator) {
        std::vector<int> values;
        for (const int value : generator) {
            values.push_back(value);
        }
        return values;
    }

    template<typename Generator>
    static void exhaust(Generator &generator) {
        for (const int value : generator) {
            (void)value;
        }
    }

    [[nodiscard]] std::vector<int> collect_fibonacci(const int count) const {
        auto generator = coroutines::fibonacci(count);
        return drain(generator);
    }

    [[nodiscard]] std::vector<int> collect_preorder() const {
        const TreeNode *const root = sample_tree();
        auto generator = coroutines::preorder(root);
        return drain(generator);
    }

    [[nodiscard]] std::vector<int> collect_inorder() const {
        const TreeNode *const root = sample_tree();
        auto generator = coroutines::inorder(root);
        return drain(generator);
    }

    [[nodiscard]] bool fibonacci_second_pass_is_exhausted() const {
        auto generator = coroutines::fibonacci(10);
        exhaust(generator);
        return drain(generator).empty();
    }
};
