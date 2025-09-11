#pragma once

// Line-oriented key/value protocol — two commands:
//   SET <key> <value>  store the pair
//   GET <key>          print the value
//
// Example (script_chunks):
//   SET name Alice  ->  OK
//   GET name        ->  Alice
//   SET age 30      ->  OK
//   GET age         ->  30

#include "common/parser/InputBuffer.h"
#include "coroutines/return_types/ResumableCoroutine.h"

#include <map>
#include <memory>
#include <span>
#include <string>
#include <string_view>

namespace coroutines {

// Shared state between the driver and the parser coroutine.
struct ProtocolParserSession {
    // The driver appends bytes here. The parser reads complete words with co_await WordAwaitable.
    parser::InputBuffer input;

    // Key/value pairs built by SET commands.
    std::map<std::string, std::string> store;
};

class ProtocolParser {
public:
    void run() const;

    // The driver pushes each chunk into the buffer and resumes the parser coroutine.
    void run_script_sync(std::span<const std::string_view> chunks) const;

    [[nodiscard]] const std::map<std::string, std::string> &store() const {
        return _session->store;
    }

private:
    // The parser coroutine pulls complete words from the buffer with co_await.
    static ResumableCoroutine parse_script(const std::shared_ptr<ProtocolParserSession> &session);

    static void store_set(
        const std::shared_ptr<ProtocolParserSession> &session,
        const std::string &key,
        const std::string &value);

    static void emit_get(const std::shared_ptr<ProtocolParserSession> &session, const std::string &key);

    std::shared_ptr<ProtocolParserSession> _session{std::make_shared<ProtocolParserSession>()};
};

}
