#include "Reactor.h"

#include <system_error>

void Reactor::run() {
    while (_running) {
        std::vector<int> write_fds = fds_for(FdMode::Write);
        std::vector<int> read_fds = fds_for(FdMode::Read);

        // Add wakeup fd to read set
        read_fds.push_back(_interrupt.arm());
        
        FdSet readFdSet(read_fds);
        FdSet writeFdSet(write_fds);

        wait_once(readFdSet, writeFdSet);

        dispatch_ready(readFdSet, writeFdSet);
    }
}

void Reactor::stop() noexcept {
    _running = false;
    _interrupt.notify();
}

void Reactor::post(int fd, FdMode mode, std::function<void(int)> handler) {
    {
        std::lock_guard<std::mutex> lock(_mtx);
        handlers_for(mode)[fd] = std::move(handler);
    }
    _interrupt.notify();
}

void Reactor::remove(int fd, FdMode mode) {
    {
        std::lock_guard<std::mutex> lock(_mtx);
        handlers_for(mode).erase(fd);
    }
    _interrupt.notify();
}

void Reactor::add_work() noexcept {
    std::lock_guard<std::mutex> lock(_mtx);
    if (!_running) {
        return;  // Don't add work if reactor is stopping
    }
    ++_work_count;
}

void Reactor::remove_work() noexcept {
    std::lock_guard<std::mutex> lock(_mtx);
    if (--_work_count == 0) {
        stop();
    }
}

auto Reactor::handlers_for(FdMode mode) -> HandlerMap& {
    return mode == FdMode::Read ? _read_handlers : _write_handlers;
}

auto Reactor::fds_for(FdMode mode) -> std::vector<int> {
    std::vector<int> fds;
    for (const auto& [fd, handler] : copy_handlers(mode)) {
        fds.push_back(fd);
    }
    return fds;
}

void Reactor::wait_once(FdSet& readFdSet, FdSet& writeFdSet) {
    auto max_fd = std::max(readFdSet.max_fd(), writeFdSet.max_fd());
    while (true) {
        int ret = ::select(max_fd + 1, readFdSet.native(), writeFdSet.native(), nullptr, nullptr);
        if (ret >= 0) {
            return;
        }
        if (errno == EINTR) {
            // retry on EINTR
            continue;
        }
        // real error
        throw std::system_error(errno, std::generic_category(), "select() failed");
    }
}

void Reactor::dispatch_ready(const FdSet& readFdSet, const FdSet& writeFdSet) {
    dispatch_ready(FdMode::Read, readFdSet);
    dispatch_ready(FdMode::Write, writeFdSet);
}

void Reactor::dispatch_ready(FdMode mode, const FdSet& readySet) {
    // Copy handlers to avoid holding lock during callbacks
    HandlerMap handlers_copy = copy_handlers(mode);

    // Dispatch without holding lock - handlers can safely call post/remove
    for (const auto& [fd, handler] : handlers_copy) {
        if (readySet.contains(fd)) {
            handler(fd);
        }
    }
}

auto Reactor::copy_handlers(FdMode mode) -> HandlerMap {
    std::lock_guard<std::mutex> lock(_mtx);
    return handlers_for(mode);
}
