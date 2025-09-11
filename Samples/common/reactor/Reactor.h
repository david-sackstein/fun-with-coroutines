#pragma once

// select()-based I/O event loop.
//
// Handlers are registered per file descriptor and direction (Read or Write).
// run() blocks the calling thread. On each iteration it calls select(), then
// invokes all handlers whose descriptors are ready. The loop stops when the
// work count (managed by WorkGuard) drops to zero.
//
// Handlers may call post() or remove() safely; dispatch copies the handler map
// before invoking callbacks so no lock is held during user code.

#include "FdSet.h"
#include "NotifySignal.h"

#include <atomic>
#include <functional>
#include <map>
#include <mutex>

class Reactor {
public:

    enum class FdMode { Read, Write };

    using HandlerMap = std::map<int, std::function<void(int)>>;

    Reactor() = default;

    // Block the calling thread and dispatch I/O events until the work count reaches zero.
    void run();

    // Stop the event loop; wakes the blocked select() call.
    void stop() noexcept;

    // Register handler to be called when fd becomes ready in the given direction.
    void post(int fd, FdMode mode, std::function<void(int)> handler);

    // Unregister the handler for fd in the given direction.
    void remove(int fd, FdMode mode);

private:
    friend class WorkGuard;

    // Increment the work count; the loop will not stop while work is positive.
    void add_work() noexcept;

    // Decrement the work count; stop the loop if it reaches zero.
    void remove_work() noexcept;

    // Return the list of registered file descriptors for the given direction.
    std::vector<int> fds_for(FdMode mode);

    // Block in select() until at least one fd is ready; retries on EINTR.
    static void wait_once(FdSet& readFdSet, FdSet& writeFdSet);

    // Dispatch ready fds for both read and write directions.
    void dispatch_ready(const FdSet& readFdSet, const FdSet& writeFdSet);

    // Invoke each handler whose fd appears in readySet.
    void dispatch_ready(FdMode mode, const FdSet& readySet);

    // Return a locked snapshot of the handler map for mode.
    HandlerMap copy_handlers(FdMode mode);

    // Return a reference to the live handler map for mode (must hold _mtx).
    HandlerMap& handlers_for(FdMode mode);

    // Separate handler maps for read and write directions.
    HandlerMap _read_handlers;
    HandlerMap _write_handlers;

    // Self-pipe used to interrupt a blocking select() when handlers change.
    NotifySignal _interrupt;
    std::atomic<bool> _running{true};
    std::atomic<int> _work_count{0};
    mutable std::mutex _mtx;
};