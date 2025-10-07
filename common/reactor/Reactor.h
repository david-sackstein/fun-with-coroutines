#pragma once

#include "FdSet.h"
#include "NotifySignal.h"

#include <map>
#include <functional>
#include <atomic>
#include <mutex>

class Reactor {
public:

    enum class FdMode { Read, Write };

    using HandlerMap = std::map<int, std::function<void(int)>>;

    Reactor() = default;

    void run();
    void stop() noexcept;

    void post(int fd, FdMode mode, std::function<void(int)> handler);
    void remove(int fd, FdMode mode);

private:
    friend class WorkGuard;

    void add_work() noexcept;
    void remove_work() noexcept;

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
    std::atomic<int> _work_count{0};
    mutable std::mutex _mtx;
};