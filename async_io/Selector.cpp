#include "Selector.h"

#include <system_error>
#include <cerrno>
#include <unistd.h>

void Selector::run() {
    while (_running) {
        FdSet fdSet(with_wakeup_fds());

        wait_once(fdSet);

        dispatch_ready(fdSet);
    }
}

void Selector::stop() noexcept {
    _running = false;
    _notify.notify();
}

void Selector::post(int fd, std::function<void(int)> handler) {
    {
        std::lock_guard<std::mutex> lock(_mtx);
        _handlers[fd] = std::move(handler);
    }
    _notify.notify();
}

void Selector::remove(int fd) {
    {
        std::lock_guard<std::mutex> lock(_mtx);
        _handlers.erase(fd);
    }
    _notify.notify();
}

auto Selector::with_wakeup_fds() -> std::vector<int> {
    std::vector<int> fds;
    for (const auto& [fd, handler] : copy_handlers()) {
        fds.push_back(fd);
    }
    fds.push_back(_notify.arm());
    return fds;
}

void Selector::wait_once(FdSet& fdSet) {
    while (true) {
        int ret = ::select(fdSet.max_fd() + 1, fdSet.native(), nullptr, nullptr, nullptr);
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

void Selector::dispatch_ready(const FdSet& fdSet) {
    // Copy handlers to avoid holding lock during callbacks
    HandlerMap handlers_copy = copy_handlers();
    
    // Dispatch without holding lock - handlers can safely call post/remove
    for (const auto& [fd, handler] : handlers_copy) {
        if (fdSet.contains(fd)) {
            handler(fd);
        }
    }
}

auto Selector::copy_handlers() const -> HandlerMap {
    std::lock_guard<std::mutex> lock(_mtx);
    return _handlers;
}
