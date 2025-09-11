#include "tests/01-generator/no-coroutines/TestGenerator.h"

TEST_F(Demo01GeneratorNoCoroutines, FibonacciSample) {
    const std::vector<int> expected{0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    EXPECT_EQ(collect_fibonacci(10), expected);
}

TEST_F(Demo01GeneratorNoCoroutines, FibonacciSecondPassExhausted) {
    EXPECT_TRUE(fibonacci_second_pass_is_exhausted());
}

TEST_F(Demo01GeneratorNoCoroutines, TreeSample) {
    EXPECT_EQ(collect_preorder(), (std::vector<int>{4, 2, 1, 3, 6, 5, 7}));
    EXPECT_EQ(collect_inorder(), (std::vector<int>{1, 2, 3, 4, 5, 6, 7}));
}

std::vector<int> Demo01GeneratorNoCoroutines::collect_fibonacci(const int count) {
    auto generator = no_coroutines::make_fibonacci(count);
    std::vector<int> values;
    while (generator.next()) {
        values.push_back(generator.get_current_value());
    }
    return values;
}

std::vector<int> Demo01GeneratorNoCoroutines::collect_preorder() {
    const TreeNode *const root = sample_tree();
    no_coroutines::PreorderTreeGenerator generator(root);
    return drain(generator);
}

std::vector<int> Demo01GeneratorNoCoroutines::collect_inorder() {
    const TreeNode *const root = sample_tree();
    no_coroutines::InorderTreeGenerator generator(root);
    return drain(generator);
}

bool Demo01GeneratorNoCoroutines::fibonacci_second_pass_is_exhausted() {
    auto generator = no_coroutines::make_fibonacci(10);
    while (generator.next()) {
        (void)generator.get_current_value();
    }
    return !generator.next();
}
