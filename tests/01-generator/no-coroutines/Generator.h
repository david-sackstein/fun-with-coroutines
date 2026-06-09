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

    template<typename Generator>
    static void exhaust(Generator &generator) {
        for (const int value : generator) {
            (void)value;
        }
    }

    [[nodiscard]] std::vector<int> collect_fibonacci(const int count) const {
        auto generator = no_coroutines::make_fibonacci(count);
        return drain(generator);
    }

    [[nodiscard]] std::vector<int> collect_preorder() const {
        const TreeNode *const root = sample_tree();
        no_coroutines::PreorderTreeGenerator generator(root);
        return drain(generator);
    }

    [[nodiscard]] std::vector<int> collect_inorder() const {
        const TreeNode *const root = sample_tree();
        no_coroutines::InorderTreeGenerator generator(root);
        return drain(generator);
    }

    [[nodiscard]] bool fibonacci_second_pass_is_exhausted() const {
        auto generator = no_coroutines::make_fibonacci(10);
        exhaust(generator);
        return drain(generator).empty();
    }
};

