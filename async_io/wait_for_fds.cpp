#include "Fd.h"
#include "FdSet.h"

#include <sys/select.h>
#include <vector>
#include <algorithm>

using namespace std::chrono;

timeval to_timeval(std::chrono::milliseconds timeout) {
    timeval tv{};
    auto msec = timeout.count();
    tv.tv_sec = msec / 1000;
    tv.tv_usec = (msec % 1000) * 1000;
    return tv;
}

std::vector<std::reference_wrapper<Fd>> wait_for_fds(
    std::vector<std::reference_wrapper<Fd>> &fds,
    std::chrono::milliseconds timeout) {

    if (fds.empty()) {
        return {};
    }

    FdSet fdSet(fds);

    timeval tv = to_timeval(timeout);

    int ret = select(fdSet.max_fd() + 1, fdSet.native(), nullptr, nullptr, &tv);
    if (ret < 0) {
        throw std::runtime_error("select() failed");
    }

    std::vector<std::reference_wrapper<Fd>> ready;
    for (auto &fd: fds) {
        if (fdSet.contains(fd)) {
            ready.push_back(fd);
        }
    }

    return ready;
}
