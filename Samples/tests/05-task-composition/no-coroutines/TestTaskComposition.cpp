#include "tests/05-task-composition/no-coroutines/TestTaskComposition.h"

#include "common/event_loop/EventLoop.h"
#include "no-coroutines/05-task-composition/CallbackChain.h"
#include "no-coroutines/05-task-composition/CalculatorBatch.h"

TEST_F(Demo05TaskCompositionNoCoroutines, CallbackChainSample) {
    EXPECT_EQ(run_callback_chain(), 60);
}

TEST_F(Demo05TaskCompositionNoCoroutines, CalculatorBatchSample) {
    EXPECT_EQ(run_calculator_batch(), (std::vector<int>{4, 7, 20, 5}));
}

int Demo05TaskCompositionNoCoroutines::run_callback_chain() {
    EventLoop loop;
    return no_coroutines::run_callback_chain(loop);
}

std::vector<int> Demo05TaskCompositionNoCoroutines::run_calculator_batch() {
    return no_coroutines::run_calculator_batch();
}
