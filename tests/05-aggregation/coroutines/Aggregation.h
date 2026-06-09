#pragma once

#include "common/event_loop/EventLoop.h"
#include "coroutines/05-aggregation/AsyncAggregate.h"
#include "coroutines/05-aggregation/CalculatorBatch.h"
#include "coroutines/05-aggregation/ParserStore.h"

#include <gtest/gtest.h>

#include <array>
#include <map>

class Demo05AggregationCoroutines : public testing::Test {
protected:
    [[nodiscard]] std::map<std::string, std::string> load_parser_store() const {
        return coroutines::load_store_from_script().get();
    }

    [[nodiscard]] int run_async_aggregate() const {
        EventLoop loop;
        coroutines::Task<int> aggregate = coroutines::aggregate_totals(loop);
        aggregate.start();
        loop.run();
        return aggregate.get();
    }

    [[nodiscard]] std::array<int, 4> run_calculator_batch() const { return coroutines::run_calculator_batch(); }
};

