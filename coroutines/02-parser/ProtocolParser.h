#pragma once

#include "common/parser/Protocol.h"
#include "coroutines/return_types/NeverSuspendCoroutine.h"

#include <map>
#include <string>
#include <string_view>

namespace coroutines {

class ProtocolParser {
public:
    void run();
    void run_script_sync();

    [[nodiscard]] const std::map<std::string, std::string> &store() const { return _store; }

private:
    NeverSuspendCoroutine run_script();
    NeverSuspendCoroutine parse_line(std::string_view line);
    NeverSuspendCoroutine handle_set(std::string_view line);
    NeverSuspendCoroutine handle_get(std::string_view line);

    std::map<std::string, std::string> _store;
};

}
