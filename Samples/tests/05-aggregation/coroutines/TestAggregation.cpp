#include "tests/05-aggregation/coroutines/TestAggregation.h"

#include "common/event_loop/EventLoop.h"
#include "coroutines/05-aggregation/AsyncAggregate.h"
#include "coroutines/05-aggregation/CalculatorBatch.h"

TEST_F(Demo05AggregationCoroutines, AsyncAggregateSample) {
    EXPECT_EQ(run_async_aggregate(), 60);
}

TEST_F(Demo05AggregationCoroutines, CalculatorBatchSample) {
    EXPECT_EQ(run_calculator_batch(), (std::array<int, 4>{4, 7, 20, 5}));
}

int Demo05AggregationCoroutines::run_async_aggregate() {
    EventLoop loop;
    coroutines::Task<int> aggregate = coroutines::aggregate_totals(loop);
    aggregate.start();
    loop.run();
    return aggregate.get();
}

std::array<int, 4> Demo05AggregationCoroutines::run_calculator_batch() {
    return coroutines::run_calculator_batch();
}
