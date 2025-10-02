#pragma once

#include "FdSet.h"
#include "NotifySignal.h"

#include <vector>
#include <sys/time.h>
#include <atomic>

class Selector {
public:

    Selector(const std::vector<int>& fds);

    [[nodiscard]] std::vector<int> wait_for_fds();
    bool is_running() const noexcept { return _running.load(); }
    void stop() noexcept;

private:

    std::vector<int> get_ready_fds(const FdSet& fdSet);
    void wait_for_fds(FdSet& fdSet);
    std::vector<int> with_wakeup_fds();

    std::vector<int> _fds;
    NotifySignal _notify;
    std::atomic<bool> _running{true};
};