#pragma once

#include "coroutines/common/Task.h"

class EventLoop;

namespace coroutines {

Task<int> aggregate_totals(EventLoop &loop);

}
