#include "tests/5. aggregation/coroutines/Demo5Aggregation.h"

TEST_F(Demo5AggregationCoroutines, ParserStoreSample) {
    const std::map<std::string, std::string> store = load_parser_store();

    ASSERT_EQ(store.size(), 2U);
    EXPECT_EQ(store.at("name"), "Alice");
    EXPECT_EQ(store.at("age"), "30");
}

TEST_F(Demo5AggregationCoroutines, AsyncAggregateSample) { EXPECT_EQ(run_async_aggregate(), 60); }

TEST_F(Demo5AggregationCoroutines, CalculatorBatchSample) {
    EXPECT_EQ(run_calculator_batch(), (std::array<int, 4>{4, 7, 20, 5}));
}
