#pragma once

#include "coroutines/2. parser/ProtocolParser.h"

#include <gtest/gtest.h>

#include <map>

class Demo2ParserCoroutines : public testing::Test {
protected:
    [[nodiscard]] std::map<std::string, std::string> run_script_store() const {
        coroutines::ProtocolParser parser;
        parser.run_script_sync();
        return parser.store();
    }
};
