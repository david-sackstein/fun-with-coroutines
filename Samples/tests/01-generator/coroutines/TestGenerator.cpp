#include "tests/01-generator/coroutines/TestGenerator.h"

TEST_F(Demo01GeneratorCoroutines, FibonacciSample) {
    const std::vector<int> expected{0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    EXPECT_EQ(collect_fibonacci(10), expected);
}

TEST_F(Demo01GeneratorCoroutines, FibonacciSecondPassExhausted) {
    EXPECT_TRUE(fibonacci_second_pass_is_exhausted());
}

TEST_F(Demo01GeneratorCoroutines, TreeSample) {
    EXPECT_EQ(collect_preorder(), (std::vector<int>{4, 2, 1, 3, 6, 5, 7}));
    EXPECT_EQ(collect_inorder(), (std::vector<int>{1, 2, 3, 4, 5, 6, 7}));
}

std::vector<int> Demo01GeneratorCoroutines::collect_fibonacci(const int count) {
    auto generator = coroutines::fibonacci(count);
    std::vector<int> values;
    while (generator.next()) {
        values.push_back(generator.get_current_value());
    }
    return values;
}

std::vector<int> Demo01GeneratorCoroutines::collect_preorder() {
    const TreeNode *const root = sample_tree();
    auto generator = coroutines::preorder(root);
    return drain(generator);
}

std::vector<int> Demo01GeneratorCoroutines::collect_inorder() {
    const TreeNode *const root = sample_tree();
    auto generator = coroutines::inorder(root);
    return drain(generator);
}

bool Demo01GeneratorCoroutines::fibonacci_second_pass_is_exhausted() {
    auto generator = coroutines::fibonacci(10);
    while (generator.next()) {
        (void)generator.get_current_value();
    }
    return !generator.next();
}
