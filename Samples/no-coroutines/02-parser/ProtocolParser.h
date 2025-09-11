#pragma once

// Line-oriented key/value protocol - two commands:
//   SET <key> <value>  store the pair
//   GET <key>          print the value
//
// Example (script_chunks):
//   SET name Alice  ->  OK
//   GET name        ->  Alice
//   SET age 30      ->  OK
//   GET age         ->  30

#include "common/parser/InputBuffer.h"

#include <map>
#include <span>
#include <string>
#include <string_view>

namespace no_coroutines {

// Push-style parser driven by an explicit state machine.
// Input arrives in chunks via feed(). Each call appends to the buffer and
// runs the state machine until it blocks waiting for more input.
// Compare with the coroutines version: the same logic is written as a
// straightforward sequential coroutine instead of an explicit Phase enum.
class ProtocolParser {
public:
    // Run the built-in demo script and print results.
    void run();

    // Feed each chunk in turn, then finish the buffer and drain remaining input.
    void run_script_sync(std::span<const std::string_view> chunks);

    // Append a chunk to the buffer and run the state machine as far as possible.
    void feed(std::string_view chunk);

    [[nodiscard]] const std::map<std::string, std::string> &store() const {
        return _store;
    }

private:
    // Each Phase corresponds to the next token the parser is waiting for.
    enum class Phase {
        Idle,        // between commands; waiting for input or end of stream
        ParseVerb,   // waiting for the command word (SET or GET)
        ParseKey,    // waiting for the key token
        ParseValue,  // waiting for the value token (SET only)
        EmitGet,     // key is ready; perform the GET lookup
    };

    // Run step() in a loop until the phase stops advancing or the buffer is empty.
    void drain();

    // Dispatch to the step function for the current phase.
    void step();

    // Try to read the command verb; advance to ParseKey on success.
    void step_parse_verb();

    // Try to read the key; advance to ParseValue (SET) or EmitGet (GET) on success.
    void step_parse_key();

    // Try to read the value; store the pair and finish the line on success.
    void step_parse_value();

    // Emit the GET result and finish the line.
    void step_emit_get();

    // Store the key/value pair and acknowledge with OK.
    void store_set(
        const std::string &key,
        const std::string &value);

    // Look up key in the store and print its value.
    void emit_get(const std::string &key);

    // Reset to Idle to begin parsing the next command.
    void finish_line();

    parser::InputBuffer _input;
    Phase _phase = Phase::Idle;

    // The most recently parsed command verb (SET or GET), held across feed() calls.
    std::string _verb;

    // The most recently parsed key, held across feed() calls.
    std::string _key;
    std::map<std::string, std::string> _store;
};

}
