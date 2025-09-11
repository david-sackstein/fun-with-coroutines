#include "Reactor.h"

#include <system_error>

// Build the fd sets and loop: wait for readiness, then dispatch handlers.
void Reactor::run() {
    while (_running) {
        std::vector<int> write_fds = fds_for(FdMode::Write);
        std::vector<int> read_fds = fds_for(FdMode::Read);

        // Include the interrupt pipe so stop() or post() can wake select().
        read_fds.push_back(_interrupt.arm());

        FdSet readFdSet(read_fds);
        FdSet writeFdSet(write_fds);

        wait_once(readFdSet, writeFdSet);

        dispatch_ready(readFdSet, writeFdSet);
    }
}

// Set the running flag to false and wake the blocked select().
void Reactor::stop() noexcept {
    _running = false;
    _interrupt.notify();
}

// Register handler and wake select() so the new fd is included in the next iteration.
void Reactor::post(const int fd, const FdMode mode, std::function<void(int)> handler) {
    {
        std::lock_guard lock(_mtx);
        handlers_for(mode)[fd] = std::move(handler);
    }
    _interrupt.notify();
}

// Unregister handler and wake select() so the removed fd is excluded in the next iteration.
void Reactor::remove(const int fd, const FdMode mode) {
    {
        std::lock_guard lock(_mtx);
        handlers_for(mode).erase(fd);
    }
    _interrupt.notify();
}

// Increment the work count so the loop stays alive.
void Reactor::add_work() noexcept {
    std::lock_guard lock(_mtx);
    if (!_running) {
        // The reactor is already stopping. Do not add work.
        return;
    }
    ++_work_count;
}

// Decrement the work count and stop the loop if it reaches zero.
void Reactor::remove_work() noexcept {
    std::lock_guard lock(_mtx);
    if (--_work_count == 0) {
        stop();
    }
}

// Collect the registered fds for mode into a plain vector (snapshot under lock).
auto Reactor::fds_for(const FdMode mode) -> std::vector<int> {
    std::vector<int> fds;
    for (const auto& [fd, handler] : copy_handlers(mode)) {
        fds.push_back(fd);
    }
    return fds;
}

// Block in select() until at least one fd is ready; retries on signal interruption.
void Reactor::wait_once(FdSet& readFdSet, FdSet& writeFdSet) {
    const auto max_fd = std::max(readFdSet.max_fd(), writeFdSet.max_fd());
    while (true) {
        const int ret = ::select(max_fd + 1, readFdSet.native(), writeFdSet.native(), nullptr, nullptr);
        if (ret >= 0) {
            return;
        }
        if (errno == EINTR) {
            // A signal interrupted select(). Retry without changing the fd sets.
            continue;
        }
        throw std::system_error(errno, std::generic_category(), "select() failed");
    }
}

// Dispatch ready events for both directions.
void Reactor::dispatch_ready(const FdSet& readFdSet, const FdSet& writeFdSet) {
    dispatch_ready(FdMode::Read, readFdSet);
    dispatch_ready(FdMode::Write, writeFdSet);
}

// Call each handler whose fd is in readySet. Uses a handler snapshot so
// the lock is not held during the callback, allowing post/remove to be called.
void Reactor::dispatch_ready(const FdMode mode, const FdSet& readySet) {
    HandlerMap handlers_copy = copy_handlers(mode);

    for (const auto& [fd, handler] : handlers_copy) {
        if (readySet.contains(fd)) {
            handler(fd);
        }
    }
}

// Return a copy of the handler map for mode, holding the lock.
auto Reactor::copy_handlers(const FdMode mode) -> HandlerMap {
    std::lock_guard lock(_mtx);
    return handlers_for(mode);
}

// Return a reference to the live handler map for mode (caller must hold _mtx).
auto Reactor::handlers_for(const FdMode mode) -> HandlerMap& {
    return mode == FdMode::Read ? _read_handlers : _write_handlers;
}
