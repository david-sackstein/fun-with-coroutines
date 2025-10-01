#include "Selector.h"

#include <system_error>
#include <cerrno>
#include <chrono>

using std::chrono::milliseconds;

Selector::Selector(FdVector &fds, std::chrono::milliseconds timeout) :
    _fds(fds),
    _timeout(timeout)
{}

auto Selector::wait_for_fds() -> FdVector {
    if (_fds.empty()) {
        return {};
    }

    // Build local fd_set each wait
    FdSet fdSet(_fds);

    wait_once(fdSet);

    return get_ready_fds(fdSet);
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

void Selector::wait_once(FdSet& fdSet) {
    for (;;) {
        timeval tv = to_timeval(_timeout);

        int ret = ::select(fdSet.max_fd() + 1, fdSet.native(), nullptr, nullptr, &tv);
        if (ret < 0 && errno == EINTR) {
            // Rebuild fd_set and retry with full timeout, matching original semantics
            fdSet = FdSet(_fds);
            continue;
        }
        if (ret < 0) {
            throw std::system_error(errno, std::generic_category(), "select() failed");
        }
        break;
    }
}

timeval Selector::to_timeval(milliseconds timeout) noexcept {
    timeval tv{};
    auto msec = timeout.count();
    tv.tv_sec = msec / 1000;
    tv.tv_usec = (msec % 1000) * 1000;
    return tv;
}
