#include "common/generator/Iterator.h"
#include "common/generator/Tree.h"
#include "coroutines/1. generator/Fibonacci.h"
#include "coroutines/1. generator/TreeTraversal.h"
#include "no-coroutines/1. generator/Fibonacci.h"
#include "no-coroutines/1. generator/TreeGenerator.h"
#include "tests/Side.h"

#include <gtest/gtest.h>

#include <vector>

namespace {

std::vector<int> collect_fibonacci(const Side side, const int count) {
    std::vector<int> values;

    if (side == Side::Coroutines) {
        auto generator = coroutines::fibonacci(count);
        for (const int value : generator) {
            values.push_back(value);
        }
        return values;
    }

    auto generator = no_coroutines::make_fibonacci(count);
    for (const int value : generator) {
        values.push_back(value);
    }
    return values;
}

std::vector<int> collect_preorder(const Side side) {
    const TreeNode *const root = sample_tree();
    std::vector<int> values;

    if (side == Side::Coroutines) {
        for (const int value : coroutines::preorder(root)) {
            values.push_back(value);
        }
        return values;
    }

    no_coroutines::PreorderTreeGenerator generator(root);
    for (const int value : generator) {
        values.push_back(value);
    }
    return values;
}

std::vector<int> collect_inorder(const Side side) {
    const TreeNode *const root = sample_tree();
    std::vector<int> values;

    if (side == Side::Coroutines) {
        for (const int value : coroutines::inorder(root)) {
            values.push_back(value);
        }
        return values;
    }

    no_coroutines::InorderTreeGenerator generator(root);
    for (const int value : generator) {
        values.push_back(value);
    }
    return values;
}

class Demo1Generator : public testing::TestWithParam<Side> {};

TEST_P(Demo1Generator, FibonacciSample) {
    const std::vector<int> expected{0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    EXPECT_EQ(collect_fibonacci(GetParam(), 10), expected);
}

TEST_P(Demo1Generator, FibonacciSecondPassExhausted) {
    if (GetParam() == Side::Coroutines) {
        auto generator = coroutines::fibonacci(10);
        for (const int value : generator) {
            (void)value;
        }
        std::vector<int> second_pass;
        for (const int value : generator) {
            second_pass.push_back(value);
        }
        EXPECT_TRUE(second_pass.empty());
        return;
    }

    auto generator = no_coroutines::make_fibonacci(10);
    for (const int value : generator) {
        (void)value;
    }
    std::vector<int> second_pass;
    for (const int value : generator) {
        second_pass.push_back(value);
    }
    EXPECT_TRUE(second_pass.empty());
}

TEST_P(Demo1Generator, TreeSample) {
    EXPECT_EQ(collect_preorder(GetParam()), (std::vector<int>{4, 2, 1, 3, 6, 5, 7}));
    EXPECT_EQ(collect_inorder(GetParam()), (std::vector<int>{1, 2, 3, 4, 5, 6, 7}));
}

INSTANTIATE_TEST_SUITE_P(Side, Demo1Generator, testing::Values(Side::Coroutines, Side::NoCoroutines),
                         [](const testing::TestParamInfo<Side> &info) { return side_name(info.param); });

}
