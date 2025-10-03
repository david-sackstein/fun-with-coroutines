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

    enum class FdMode { Read, Write };

    using HandlerMap = std::map<int, std::function<void(int)>>;

    Selector() = default;

    void run();
    void stop() noexcept;

    void post(int fd, FdMode mode, std::function<void(int)> handler);
    void remove(int fd, FdMode mode);

private:

    HandlerMap& handlers_for(FdMode mode);
    std::vector<int> fds_for(FdMode mode);

    void wait_once(FdSet& readFdSet, FdSet& writeFdSet);

    void dispatch_ready(const FdSet& readFdSet, const FdSet& writeFdSet);
    void dispatch_ready(FdMode mode, const FdSet& readySet);

    HandlerMap copy_handlers(FdMode mode);

    HandlerMap _read_handlers;
    HandlerMap _write_handlers;

    NotifySignal _interrupt;
    std::atomic<bool> _running{true};
    mutable std::mutex _mtx;
};