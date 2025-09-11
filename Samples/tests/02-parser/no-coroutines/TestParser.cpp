#include "tests/02-parser/no-coroutines/TestParser.h"

#include <array>

std::map<std::string, std::string> Demo02ParserNoCoroutines::run_script_store() {
    constexpr std::array<std::string_view, 6> script_chunks = {
        "SET na",
        "me Alice\n",
        "GET na",
        "me\n",
        "SET age ",
        "30\nGET age\n",
    };

    no_coroutines::ProtocolParser parser;
    parser.run_script_sync(script_chunks);
    return parser.store();
}

TEST_F(Demo02ParserNoCoroutines, ScriptBuildsStore) {
    const std::map<std::string, std::string> store = run_script_store();

    ASSERT_EQ(store.size(), 2U);
    EXPECT_EQ(store.at("name"), "Alice");
    EXPECT_EQ(store.at("age"), "30");
}
