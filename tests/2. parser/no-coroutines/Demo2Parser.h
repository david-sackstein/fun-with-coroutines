#pragma once

#include "no-coroutines/2. parser/LineProtocolParser.h"

#include <gtest/gtest.h>

#include <map>

class Demo2ParserNoCoroutines : public testing::Test {
protected:
    [[nodiscard]] std::map<std::string, std::string> run_script_store() const {
        no_coroutines::LineProtocolParser parser;
        parser.run();
        return parser.store();
    }
};
