#pragma once

#include "Fd.h"
#include "FdSet.h"

class Selector {
public:

    using FdVector = std::vector<std::reference_wrapper<Fd>>;

    static FdVector wait_for_fds(FdVector &fds, std::chrono::milliseconds timeout);

    static FdVector get_ready_fds(FdVector &fds, FdSet &fdSet);

    static timeval to_timeval(std::chrono::milliseconds timeout);

    static void select(FdSet &fdSet, std::chrono::milliseconds);
};