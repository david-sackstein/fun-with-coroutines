#pragma once

#include "FdSet.h"
#include "NotifySignal.h"

#include <vector>
#include <functional>
#include <sys/time.h>
#include <atomic>

class Selector {
public:

    struct FdHandler {
        int fd;
        std::function<void(int)> handler;
    };

    Selector(const std::vector<FdHandler>& handlers);

    void run();
    void stop() noexcept;

private:

    void wait_once(FdSet& fdSet);
    std::vector<int> with_wakeup_fds();
    void dispatch_ready(const FdSet& fdSet);

    std::vector<FdHandler> _handlers;
    NotifySignal _notify;
    std::atomic<bool> _running{true};
};