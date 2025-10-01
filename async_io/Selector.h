#pragma once

#include "Fd.h"
#include "FdSet.h"

class Selector {
public:

    using FdVector = std::vector<std::reference_wrapper<Fd>>;

    Selector(FdVector &fds, std::chrono::milliseconds timeout);

    FdVector wait_for_fds();

private:

    FdVector get_ready_fds();
    void select();

    static timeval to_timeval(std::chrono::milliseconds timeout);

    FdSet _fdSet;
    FdVector _fds;
    std::chrono::milliseconds _timeout;
};