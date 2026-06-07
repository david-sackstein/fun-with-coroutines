#include "tests/1. generator/coroutines/Demo1Generator.h"

TEST_F(Demo1GeneratorCoroutines, FibonacciSample) {
    const std::vector<int> expected{0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    EXPECT_EQ(collect_fibonacci(10), expected);
}

TEST_F(Demo1GeneratorCoroutines, FibonacciSecondPassExhausted) { EXPECT_TRUE(fibonacci_second_pass_is_exhausted()); }

TEST_F(Demo1GeneratorCoroutines, TreeSample) {
    EXPECT_EQ(collect_preorder(), (std::vector<int>{4, 2, 1, 3, 6, 5, 7}));
    EXPECT_EQ(collect_inorder(), (std::vector<int>{1, 2, 3, 4, 5, 6, 7}));
}
