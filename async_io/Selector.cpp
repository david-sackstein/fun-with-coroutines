#include "Selector.h"

using namespace std::chrono;

Selector::Selector(FdVector &fds, std::chrono::milliseconds timeout) :
    _fdSet(fds),
    _fds(fds),
    _timeout(timeout)
{
}

auto Selector::wait_for_fds() -> FdVector {

    if (_fds.empty()) {
        return {};
    }

    select();

    return get_ready_fds();
}

auto Selector::get_ready_fds() -> FdVector {
    FdVector ready{};

    for (auto &fd: _fds) {
        if (_fdSet.contains(fd)) {
            ready.push_back(fd);
        }
    }

    return ready;
}

void Selector::select() {
    timeval tv = to_timeval(_timeout);

    int ret = ::select(_fdSet.max_fd() + 1, _fdSet.native(), nullptr, nullptr, &tv);
    if (ret < 0) {
        throw std::system_error(errno, std::generic_category(), "select() failed");
    }
}

timeval Selector::to_timeval(milliseconds timeout) {
    timeval tv{};
    auto msec = timeout.count();
    tv.tv_sec = msec / 1000;
    tv.tv_usec = (msec % 1000) * 1000;
    return tv;
}
