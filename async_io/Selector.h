#pragma once

#include "Fd.h"
#include "FdSet.h"
#include "NotifySignal.h"

#include <vector>
#include <functional>
#include <sys/time.h>
#include <atomic>

class Selector {
public:

    using FdVector = std::vector<std::reference_wrapper<Fd>>;

    Selector(FdVector &fds);

    [[nodiscard]] FdVector wait_for_fds();
    bool is_running() const noexcept { return _running.load(); }
    void stop() noexcept;

private:

    FdVector get_ready_fds(const FdSet& fdSet);
    void wait_for_fds(FdSet& fdSet);
    FdVector with_wakeup_fds();

    FdVector _fds;
    NotifySignal _notify;
    std::atomic<bool> _running{true};
};