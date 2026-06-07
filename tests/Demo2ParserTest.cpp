#include "coroutines/2. parser/ProtocolParser.h"
#include "no-coroutines/2. parser/LineProtocolParser.h"
#include "tests/Side.h"

#include <gtest/gtest.h>

#include <map>
#include <sstream>
#include <streambuf>
#include <string>

namespace {

std::map<std::string, std::string> run_parser(const Side side) {
    if (side == Side::Coroutines) {
        coroutines::ProtocolParser parser;
        parser.run_script_sync();
        return parser.store();
    }

    no_coroutines::LineProtocolParser parser;
    parser.run();
    return parser.store();
}

class Demo2Parser : public testing::TestWithParam<Side> {};

TEST_P(Demo2Parser, ScriptBuildsStore) {
    const std::map<std::string, std::string> store = run_parser(GetParam());

    ASSERT_EQ(store.size(), 2U);
    EXPECT_EQ(store.at("name"), "Alice");
    EXPECT_EQ(store.at("age"), "30");
}

INSTANTIATE_TEST_SUITE_P(Side, Demo2Parser, testing::Values(Side::Coroutines, Side::NoCoroutines),
                         [](const testing::TestParamInfo<Side> &info) { return side_name(info.param); });

}
