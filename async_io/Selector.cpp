#include "Selector.h"

#include <system_error>
#include <cerrno>
#include <unistd.h>

void Selector::run() {
    while (_running) {
        FdSet readFdSet(read_with_wakeup_fds());
        FdSet writeFdSet(write_fds());

        wait_once(readFdSet, writeFdSet);

        dispatch_ready(readFdSet, writeFdSet);
    }
}

void Selector::stop() noexcept {
    _running = false;
    _notify.notify();
}

void Selector::post_read(int fd, std::function<void(int)> handler) {
    {
        std::lock_guard<std::mutex> lock(_mtx);
        _read_handlers[fd] = std::move(handler);
    }
    _notify.notify();
}

void Selector::remove_read(int fd) {
    {
        std::lock_guard<std::mutex> lock(_mtx);
        _read_handlers.erase(fd);
    }
    _notify.notify();
}

void Selector::post_write(int fd, std::function<void(int)> handler) {
    {
        std::lock_guard<std::mutex> lock(_mtx);
        _write_handlers[fd] = std::move(handler);
    }
    _notify.notify();
}

void Selector::remove_write(int fd) {
    {
        std::lock_guard<std::mutex> lock(_mtx);
        _write_handlers.erase(fd);
    }
    _notify.notify();
}

auto Selector::read_with_wakeup_fds() -> std::vector<int> {
    std::vector<int> fds;
    for (const auto& [fd, handler] : copy_read_handlers()) {
        fds.push_back(fd);
    }
    fds.push_back(_notify.arm());
    return fds;
}

auto Selector::write_fds() -> std::vector<int> {
    std::vector<int> fds;
    for (const auto& [fd, handler] : copy_write_handlers()) {
        fds.push_back(fd);
    }
    return fds;
}

void Selector::wait_once(FdSet& readFdSet, FdSet& writeFdSet) {
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

void Selector::dispatch_ready(const FdSet& readFdSet, const FdSet& writeFdSet) {
    // Copy handlers to avoid holding lock during callbacks
    HandlerMap read_handlers_copy = copy_read_handlers();
    
    // Dispatch without holding lock - handlers can safely call post_read/post_write/remove
    for (const auto& [fd, handler] : read_handlers_copy) {
        if (readFdSet.contains(fd)) {
            handler(fd);
        }
    }

    HandlerMap write_handlers_copy = copy_write_handlers();

    for (const auto& [fd, handler] : write_handlers_copy) {
        if (writeFdSet.contains(fd)) {
            handler(fd);
        }
    }
}

auto Selector::copy_read_handlers() const -> HandlerMap {
    std::lock_guard<std::mutex> lock(_mtx);
    return _read_handlers;
}

auto Selector::copy_write_handlers() const -> HandlerMap {
    std::lock_guard<std::mutex> lock(_mtx);
    return _write_handlers;
}
