#pragma once

#include "coroutines/return_types/Task.h"

class EventLoop;

namespace coroutines {

Task<int> aggregate_totals(EventLoop &loop);

}
