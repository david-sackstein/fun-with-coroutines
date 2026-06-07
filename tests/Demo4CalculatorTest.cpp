#include "tests/CalcSession.h"
#include "tests/Side.h"

#include <gtest/gtest.h>

#include <string>

namespace {

class Demo4Calculator : public testing::TestWithParam<Side> {};

TEST_P(Demo4Calculator, PipedExpression) {
    const std::string output = test_helpers::run_calculator_session(GetParam(), "23+5\n");

    EXPECT_NE(output.find("28"), std::string::npos);
    EXPECT_NE(output.find("Response verified successfully"), std::string::npos);
}

INSTANTIATE_TEST_SUITE_P(Side, Demo4Calculator, testing::Values(Side::Coroutines, Side::NoCoroutines),
                         [](const testing::TestParamInfo<Side> &info) { return side_name(info.param); });

}
