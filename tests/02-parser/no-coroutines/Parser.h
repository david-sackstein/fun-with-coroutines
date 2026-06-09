#pragma once

#include "no-coroutines/02-parser/ProtocolParser.h"

#include <gtest/gtest.h>
#include <map>

class Demo02ParserNoCoroutines : public testing::Test {
protected:
    [[nodiscard]] std::map<std::string, std::string> run_script_store() const {
        no_coroutines::ProtocolParser parser;
        parser.run();
        return parser.store();
    }
};

