#pragma once

#include "no-coroutines/02-parser/ProtocolParser.h"

#include <gtest/gtest.h>
#include <map>

class Demo02ParserNoCoroutines : public testing::Test {
protected:
    [[nodiscard]] static std::map<std::string, std::string> run_script_store();
};
