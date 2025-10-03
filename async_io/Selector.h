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

    void post_read(int fd, std::function<void(int)> handler);
    void remove_read(int fd);

    void post_write(int fd, std::function<void(int)> handler);
    void remove_write(int fd);

private:

    std::vector<int> read_with_wakeup_fds();
    std::vector<int> write_fds();

    void wait_once(FdSet& readFdSet, FdSet& writeFdSet);
    void dispatch_ready(const FdSet& readFdSet, const FdSet& writeFdSet);

    HandlerMap copy_read_handlers() const;
    HandlerMap copy_write_handlers() const;

    HandlerMap _read_handlers;
    HandlerMap _write_handlers;
    NotifySignal _notify;
    std::atomic<bool> _running{true};
    mutable std::mutex _mtx;
};