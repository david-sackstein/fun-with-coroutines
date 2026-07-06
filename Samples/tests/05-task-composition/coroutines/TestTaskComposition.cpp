#include "tests/05-task-composition/coroutines/TestTaskComposition.h"

#include "coroutines/05-task-composition/CalculatorBatch.h"
#include "coroutines/05-task-composition/TaskChain.h"

TEST_F(Demo05TaskCompositionCoroutines, TaskChain) {
    EXPECT_EQ(run_task_chain(), 60);
}

TEST_F(Demo05TaskCompositionCoroutines, CalculatorBatchSample) {
    EXPECT_EQ(run_calculator_batch(), (std::vector<int>{4, 7, 20, 5}));
}

int Demo05TaskCompositionCoroutines::run_task_chain() {
    return coroutines::run_task_chain();
}

std::vector<int> Demo05TaskCompositionCoroutines::run_calculator_batch() {
    return coroutines::run_calculator_batch();
}
