#include "Selector.h"

using namespace std::chrono;

auto Selector::wait_for_fds(FdVector &fds, milliseconds timeout) -> FdVector {

    if (fds.empty()) {
        return {};
    }

    FdSet fdSet(fds);

    select(fdSet, timeout);

    return get_ready_fds(fds, fdSet);
}

auto Selector::get_ready_fds(FdVector &fds, FdSet &fdSet) -> FdVector{
    FdVector ready;
    for (auto &fd: fds) {
        if (fdSet.contains(fd)) {
            ready.push_back(fd);
        }
    }
    return ready;
}

void Selector::select(FdSet &fdSet, milliseconds timeout){
    timeval tv = to_timeval(timeout);

    int ret = ::select(fdSet.max_fd() + 1, fdSet.native(), nullptr, nullptr, &tv);
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
