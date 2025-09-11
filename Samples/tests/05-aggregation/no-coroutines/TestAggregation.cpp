#include "tests/05-aggregation/no-coroutines/TestAggregation.h"

#include "common/event_loop/EventLoop.h"
#include "no-coroutines/05-aggregation/AsyncAggregate.h"
#include "no-coroutines/05-aggregation/CalculatorBatch.h"

TEST_F(Demo05AggregationNoCoroutines, AsyncAggregateSample) {
    EXPECT_EQ(run_async_aggregate(), 60);
}

TEST_F(Demo05AggregationNoCoroutines, CalculatorBatchSample) {
    EXPECT_EQ(run_calculator_batch(), (std::array<int, 4>{4, 7, 20, 5}));
}

int Demo05AggregationNoCoroutines::run_async_aggregate() {
    EventLoop loop;
    return no_coroutines::run_async_aggregate(loop);
}

std::array<int, 4> Demo05AggregationNoCoroutines::run_calculator_batch() {
    return no_coroutines::run_calculator_batch();
}
