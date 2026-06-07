#pragma once

#include "coroutines/common/Task.h"

#include <map>
#include <string>

namespace coroutines {

Task<std::map<std::string, std::string>> load_store_from_script();

}
