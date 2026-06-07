#include "common/event_loop/EventLoop.h"
#include "coroutines/5. aggregation/AsyncAggregate.h"
#include "coroutines/5. aggregation/CalculatorBatch.h"
#include "coroutines/5. aggregation/ParserStore.h"
#include "no-coroutines/5. aggregation/AsyncAggregate.h"
#include "no-coroutines/5. aggregation/CalculatorBatch.h"
#include "no-coroutines/5. aggregation/ParserStore.h"
#include "tests/Side.h"

#include <gtest/gtest.h>

#include <array>
#include <map>

namespace {

std::map<std::string, std::string> load_parser_store(const Side side) {
    if (side == Side::Coroutines) {
        return coroutines::load_store_from_script().get();
    }
    return no_coroutines::load_store_from_script();
}

int run_async_aggregate(const Side side) {
    EventLoop loop;
    if (side == Side::Coroutines) {
        coroutines::Task<int> aggregate = coroutines::aggregate_totals(loop);
        aggregate.start();
        loop.run();
        return aggregate.get();
    }
    return no_coroutines::run_async_aggregate(loop);
}

std::array<int, 4> run_calculator_batch(const Side side) {
    if (side == Side::Coroutines) {
        return coroutines::run_calculator_batch();
    }
    return no_coroutines::run_calculator_batch();
}

class Demo5Aggregation : public testing::TestWithParam<Side> {};

TEST_P(Demo5Aggregation, ParserStoreSample) {
    const std::map<std::string, std::string> store = load_parser_store(GetParam());

    ASSERT_EQ(store.size(), 2U);
    EXPECT_EQ(store.at("name"), "Alice");
    EXPECT_EQ(store.at("age"), "30");
}

TEST_P(Demo5Aggregation, AsyncAggregateSample) {
    EXPECT_EQ(run_async_aggregate(GetParam()), 60);
}

TEST_P(Demo5Aggregation, CalculatorBatchSample) {
    EXPECT_EQ(run_calculator_batch(GetParam()), (std::array<int, 4>{4, 7, 20, 5}));
}

INSTANTIATE_TEST_SUITE_P(Side, Demo5Aggregation, testing::Values(Side::Coroutines, Side::NoCoroutines),
                         [](const testing::TestParamInfo<Side> &info) { return side_name(info.param); });

}
