#pragma once

#include "common/parser/Protocol.h"
#include "coroutines/common/CoroutineObject.h"

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
    CoroutineObject run_script();
    CoroutineObject parse_line(std::string_view line);
    CoroutineObject handle_set(std::string_view line);
    CoroutineObject handle_get(std::string_view line);

    std::map<std::string, std::string> _store;
};

}
