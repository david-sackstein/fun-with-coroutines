#include "common/io/print.h"
#include "coroutines/02-parser/ProtocolParser.h"
#include "coroutines/02-parser/WordAwaitable.h"

#include <array>

namespace coroutines {

void ProtocolParser::run() const {
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

// The driver pushes each chunk into the buffer and resumes the parser coroutine.
void ProtocolParser::run_script_sync(const std::span<const std::string_view> chunks) const {
    // Create the parser coroutine in a suspended state.
    ResumableCoroutine script = parse_script(_session);
    for (const std::string_view chunk : chunks) {
        _session->input.append(chunk);
        if (!script.done()) {
            // The parser is waiting for more input. Resume it now that the buffer grew.
            script.resume();
        }
    }
    // All chunks have been fed. Mark the buffer finished so a trailing partial token can complete.
    _session->input.finish();
    while (!script.done()) {
        // Input is finished. Keep resuming until the parser reaches co_return.
        script.resume();
    }
}

// The parser coroutine pulls complete words from the buffer with co_await.
ResumableCoroutine ProtocolParser::parse_script(const std::shared_ptr<ProtocolParserSession> &session) {
    while (true) {
        const std::optional<std::string> command_word = co_await WordAwaitable{session->input};
        if (!command_word) {
            // Input ended before a complete command word.
            co_return;
        }

        // Command token is ready (SET or GET).
        const std::string& command = *command_word;

        if (command == "SET") {
            // SET key value
            const std::optional<std::string> key_word = co_await WordAwaitable{session->input};
            if (!key_word) {
                // Input ended before the SET key.
                co_return;
            }

            // Key token is ready.
            const std::string& key = *key_word;

            const std::optional<std::string> value_word = co_await WordAwaitable{session->input};
            if (!value_word) {
                // Input ended before the SET value.
                co_return;
            }
            const std::string& value = *value_word;

            // All SET tokens are ready. Store the pair and read the next command.
            store_set(session, key, value);
            continue;
        }

        if (command == "GET") {
            // GET key
            const std::optional<std::string> key_word = co_await WordAwaitable{session->input};
            if (!key_word) {
                // Input ended before the GET key.
                co_return;
            }

            // Lookup key token is ready.
            const std::string& key = *key_word;

            // Print the stored value for this key.
            emit_get(session, key);
        }
    }
}

// Save the pair and acknowledge with OK.
void ProtocolParser::store_set(
    const std::shared_ptr<ProtocolParserSession> &session,
    const std::string &key,
    const std::string &value) {
    session->store.emplace(key, value);
    io::print("OK\n");
}

// Look up key in the store and print its value.
void ProtocolParser::emit_get(const std::shared_ptr<ProtocolParserSession> &session, const std::string &key) {
    const auto iterator = session->store.find(key);
    if (iterator != session->store.end()) {
        // The key is in the store. Print its value.
        io::print("{}\n", iterator->second);
    }
}

}
