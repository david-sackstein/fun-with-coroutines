#pragma once

#include "FileDescriptor.h"
#include <set>
#include <vector>
#include <chrono>

std::vector<std::reference_wrapper<FileDescriptor>> wait_for_fds(std::vector<std::reference_wrapper<FileDescriptor>>& fds,
                                                                 std::chrono::milliseconds timeout);