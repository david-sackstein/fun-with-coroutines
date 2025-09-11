#include "common/io/print.h"
#include "no-coroutines/02-parser/ProtocolParser.h"

#include <array>

namespace no_coroutines {

// Print the banner and run the built-in demo script.
void ProtocolParser::run() {
    io::print("=== Parser protocol demo ===\n");
    io::print("Commands: SET <key> <value>, GET <key>\n\n");

    constexpr std::array<std::string_view, 6> script_chunks = {
        "SET na",
        "me Alice\n",
        "GET na",
        "me\n",
        "SET age ",
        "30\nGET age\n",
    };

    run_script_sync(script_chunks);
}

// Feed each chunk in sequence, then mark input finished and drain any remaining tokens.
void ProtocolParser::run_script_sync(const std::span<const std::string_view> chunks) {
    for (const std::string_view chunk : chunks) {
        feed(chunk);
    }
    // Signal end of input so a trailing token without a delimiter can complete.
    _input.finish();
    drain();
}

// Append the chunk to the buffer and advance the state machine as far as possible.
void ProtocolParser::feed(const std::string_view chunk) {
    _input.append(chunk);
    drain();
}

// Advance the state machine until a step blocks waiting for more input.
void ProtocolParser::drain() {
    while (true) {
        if (_phase == Phase::Idle) {
            if (_input.at_end()) {
                // Input is finished and the buffer is empty. All commands processed.
                return;
            }
            if (!_input.word_ready()) {
                // No complete token yet. Wait for the next feed().
                return;
            }
            // A token is ready. Begin parsing the next command.
            _phase = Phase::ParseVerb;
        }

        const Phase previous = _phase;
        step();

        if (_phase == previous) {
            // The step did not advance. The next token is incomplete. Wait for the next feed().
            return;
        }
    }
}

// Dispatch to the handler for the current phase.
void ProtocolParser::step() {
    switch (_phase) {
    case Phase::Idle:
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
    }
}

// Try to read the command verb. Store it and advance to ParseKey on success.
void ProtocolParser::step_parse_verb() {
    const std::optional<std::string> command = _input.take_word();
    if (!command) {
        // Token incomplete. Remain in ParseVerb until the next feed().
        return;
    }
    _verb = *command;
    _phase = Phase::ParseKey;
}

// Try to read the key. Store it and advance to the next phase based on the verb.
void ProtocolParser::step_parse_key() {
    const std::optional<std::string> key = _input.take_word();
    if (!key) {
        // Token incomplete. Remain in ParseKey until the next feed().
        return;
    }
    _key = *key;
    if (_verb == "SET") {
        // SET needs a value next.
        _phase = Phase::ParseValue;
    } else if (_verb == "GET") {
        // GET has all tokens. Emit the result.
        _phase = Phase::EmitGet;
    } else {
        // Unknown verb. Skip this command.
        finish_line();
    }
}

// Try to read the value. Store the pair and finish the line on success.
void ProtocolParser::step_parse_value() {
    const std::optional<std::string> value = _input.take_word();
    if (!value) {
        // Token incomplete. Remain in ParseValue until the next feed().
        return;
    }
    store_set(_key, *value);
    finish_line();
}

// Emit the GET result for the stored key and finish the line.
void ProtocolParser::step_emit_get() {
    emit_get(_key);
    finish_line();
}

// Save the key/value pair and acknowledge with OK.
void ProtocolParser::store_set(
    const std::string &key,
    const std::string &value) {
    _store[key] = value;
    io::print("OK\n");
}

// Look up key in the store and print its value.
void ProtocolParser::emit_get(const std::string &key) {
    const auto iterator = _store.find(key);
    if (iterator != _store.end()) {
        // The key is in the store. Print its value.
        io::print("{}\n", iterator->second);
    }
}

// Reset the phase to Idle to begin parsing the next command.
void ProtocolParser::finish_line() {
    _phase = Phase::Idle;
}

}
