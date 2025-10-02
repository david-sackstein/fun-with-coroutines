#pragma once

#include "FdSet.h"
#include "NotifySignal.h"

#include <map>
#include <functional>
#include <sys/time.h>
#include <atomic>
#include <mutex>

class Selector {
public:

    using HandlerMap = std::map<int, std::function<void(int)>>;

    Selector() = default;

    void run();
    void stop() noexcept;
    void post(int fd, std::function<void(int)> handler);
    void remove(int fd);

private:

    std::vector<int> with_wakeup_fds();
    void wait_once(FdSet& fdSet);
    void dispatch_ready(const FdSet& fdSet);
    HandlerMap copy_handlers() const;

    HandlerMap _handlers;
    NotifySignal _notify;
    std::atomic<bool> _running{true};
    mutable std::mutex _mtx;
};