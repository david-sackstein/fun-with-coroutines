#include "tests/2. parser/coroutines/Demo2Parser.h"

TEST_F(Demo2ParserCoroutines, ScriptBuildsStore) {
    const std::map<std::string, std::string> store = run_script_store();

    ASSERT_EQ(store.size(), 2U);
    EXPECT_EQ(store.at("name"), "Alice");
    EXPECT_EQ(store.at("age"), "30");
}
