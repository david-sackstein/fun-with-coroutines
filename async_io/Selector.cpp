#include "Selector.h"

#include <system_error>
#include <cerrno>
#include <unistd.h>

Selector::Selector(const std::vector<int>& fds) :
    _fds(fds),
    _notify()
{}

auto Selector::wait_for_fds() -> std::vector<int> {
    if (_fds.empty() || !_running) {
        return {};
    }

    FdSet fdSet(with_wakeup_fds());

    wait_for_fds(fdSet);

    return get_ready_fds(fdSet);
}

void Selector::stop() noexcept {
    if (_running) {
        _notify.notify();
    }
}

auto Selector::get_ready_fds(const FdSet& fdSet) -> std::vector<int> {
    std::vector<int> ready{};

    for (int fd : _fds) {
        if (fdSet.contains(fd)) {
            ready.push_back(fd);
        }
    }

    return ready;
}

void Selector::wait_for_fds(FdSet& fdSet) {
    for (;;) {
        int ret = ::select(fdSet.max_fd() + 1, fdSet.native(), nullptr, nullptr, nullptr);
        if (ret < 0 && errno == EINTR) {
            // Rebuild fd_set and retry
            fdSet = FdSet(with_wakeup_fds());
            continue;
        }
        if (ret < 0) {
            throw std::system_error(errno, std::generic_category(), "select() failed");
        }
        break;
    }
}

auto Selector::with_wakeup_fds() -> std::vector<int> {
    std::vector<int> list = _fds;
    list.push_back(_notify.arm());
    return list;
}