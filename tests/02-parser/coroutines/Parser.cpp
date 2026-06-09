#include "tests/02-parser/coroutines/Parser.h"

TEST_F(Demo02ParserCoroutines, ScriptBuildsStore) {
    const std::map<std::string, std::string> store = run_script_store();

    ASSERT_EQ(store.size(), 2U);
    EXPECT_EQ(store.at("name"), "Alice");
    EXPECT_EQ(store.at("age"), "30");
}

