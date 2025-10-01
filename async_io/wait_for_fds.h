#pragma once

#include "Fd.h"
#include <set>
#include <vector>
#include <chrono>

std::vector<std::reference_wrapper<Fd>> wait_for_fds(std::vector<std::reference_wrapper<Fd>>& fds,
                                                     std::chrono::milliseconds timeout);