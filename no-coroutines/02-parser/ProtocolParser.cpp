#include "common/io/print.h"
#include "no-coroutines/02-parser/ProtocolParser.h"

namespace no_coroutines {

void ProtocolParser::run() {
    while (_phase != Phase::Done) {
        step();
    }
}

void ProtocolParser::step() {
    switch (_phase) {
    case Phase::ReadLine:
        step_read_line();
        return;
    case Phase::ParseVerb:
        step_parse_verb();
        return;
    case Phase::ParseKey:
        step_parse_key();
        return;
    case Phase::ParseValue:
        step_parse_value();
        return;
    case Phase::EmitGet:
        step_emit_get();
        return;
    case Phase::Done:
        return;
    }
}

void ProtocolParser::step_read_line() {
    if (_line_index >= parser::script_lines.size()) {
        _phase = Phase::Done;
        return;
    }
    _rest = parser::script_lines[_line_index];
    _verb = {};
    _phase = Phase::ParseVerb;
}

void ProtocolParser::step_parse_verb() {
    const auto command = parser::take_word(_rest);
    if (!command) {
        advance_line();
        return;
    }
    _verb = std::string(*command);
    _phase = Phase::ParseKey;
}

void ProtocolParser::step_parse_key() {
    const auto key = parser::take_word(_rest);
    if (!key) {
        advance_line();
        return;
    }
    _key = std::string(*key);
    if (_verb == "SET") {
        _phase = Phase::ParseValue;
    } else if (_verb == "GET") {
        _phase = Phase::EmitGet;
    } else {
        advance_line();
    }
}

void ProtocolParser::step_parse_value() {
    const auto value = parser::take_word(_rest);
    if (!value) {
        advance_line();
        return;
    }
    _store[_key] = std::string(*value);
    io::print("OK\n");
    advance_line();
}

void ProtocolParser::step_emit_get() {
    const auto iterator = _store.find(_key);
    if (iterator != _store.end()) {
        io::print("{}\n", iterator->second);
    }
    advance_line();
}

void ProtocolParser::advance_line() {
    ++_line_index;
    _phase = Phase::ReadLine;
}

}
