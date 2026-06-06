#pragma once

#include "common/parser/Protocol.h"

#include <map>
#include <string>
#include <string_view>

namespace no_coroutines {

class LineProtocolParser {
public:
    void run();

private:
    enum class Phase {
        ReadLine,
        ParseVerb,
        ParseKey,
        ParseValue,
        EmitGet,
        Done,
    };

    void step();
    void step_read_line();
    void step_parse_verb();
    void step_parse_key();
    void step_parse_value();
    void step_emit_get();
    void advance_line();

    Phase _phase = Phase::ReadLine;
    size_t _line_index = 0;
    std::string_view _rest;
    std::string _verb;
    std::string _key;
    std::map<std::string, std::string> _store;
};

}
