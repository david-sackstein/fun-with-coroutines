#pragma once

#include <map>
#include <string>

namespace no_coroutines {

[[nodiscard]] std::map<std::string, std::string> load_store_from_script();

}
