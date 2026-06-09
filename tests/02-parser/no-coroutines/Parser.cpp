#include "tests/02-parser/no-coroutines/Parser.h"

TEST_F(Demo02ParserNoCoroutines, ScriptBuildsStore) {
    const std::map<std::string, std::string> store = run_script_store();

    ASSERT_EQ(store.size(), 2U);
    EXPECT_EQ(store.at("name"), "Alice");
    EXPECT_EQ(store.at("age"), "30");
}

