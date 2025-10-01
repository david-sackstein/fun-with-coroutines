#pragma once

#include "Fd.h"
#include "FdSet.h"

#include <vector>
#include <functional>
#include <chrono>
#include <sys/time.h>

class Selector {
public:

    using FdVector = std::vector<std::reference_wrapper<Fd>>;

    Selector(FdVector &fds, std::chrono::milliseconds timeout);

    [[nodiscard]] FdVector wait_for_fds();

private:

    FdVector get_ready_fds(const FdSet& fdSet);
    void wait_once(FdSet& fdSet);

    static timeval to_timeval(std::chrono::milliseconds timeout) noexcept;

    FdVector _fds;
    std::chrono::milliseconds _timeout;
};