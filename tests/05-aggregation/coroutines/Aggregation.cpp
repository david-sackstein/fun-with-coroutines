#include "tests/05-aggregation/coroutines/Aggregation.h"

TEST_F(Demo05AggregationCoroutines, ParserStoreSample) {
    const std::map<std::string, std::string> store = load_parser_store();

    ASSERT_EQ(store.size(), 2U);
    EXPECT_EQ(store.at("name"), "Alice");
    EXPECT_EQ(store.at("age"), "30");
}

TEST_F(Demo05AggregationCoroutines, AsyncAggregateSample) { EXPECT_EQ(run_async_aggregate(), 60); }

TEST_F(Demo05AggregationCoroutines, CalculatorBatchSample) {
    EXPECT_EQ(run_calculator_batch(), (std::array<int, 4>{4, 7, 20, 5}));
}

