#include "Selector.h"

#include <system_error>
#include <cerrno>
#include <unistd.h>

Selector::Selector(FdVector &fds) :
    _fds(fds),
    _notify()
{}

auto Selector::wait_for_fds() -> FdVector {
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

auto Selector::get_ready_fds(const FdSet& fdSet) -> FdVector {
    FdVector ready{};

    for (auto &fd: _fds) {
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

auto Selector::with_wakeup_fds() -> FdVector {
    FdVector list = _fds;
    list.push_back(std::ref(_notify.arm()));
    return list;
}