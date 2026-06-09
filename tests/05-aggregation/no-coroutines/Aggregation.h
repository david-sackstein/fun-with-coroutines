#pragma once

#include "common/event_loop/EventLoop.h"
#include "no-coroutines/05-aggregation/AsyncAggregate.h"
#include "no-coroutines/05-aggregation/CalculatorBatch.h"
#include "no-coroutines/05-aggregation/ParserStore.h"

#include <gtest/gtest.h>

#include <array>
#include <map>

class Demo05AggregationNoCoroutines : public testing::Test {
protected:
    [[nodiscard]] std::map<std::string, std::string> load_parser_store() const {
        return no_coroutines::load_store_from_script();
    }

    [[nodiscard]] int run_async_aggregate() const {
        EventLoop loop;
        return no_coroutines::run_async_aggregate(loop);
    }

    [[nodiscard]] std::array<int, 4> run_calculator_batch() const { return no_coroutines::run_calculator_batch(); }
};

