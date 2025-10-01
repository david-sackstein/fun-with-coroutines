#include "wait_for_fds.h"
#include "FileDescriptor.h"
#include "FdSet.h"

#include <sys/select.h>
#include <vector>
#include <chrono>
#include <set>
#include <stdexcept>
#include <algorithm>


std::vector<std::reference_wrapper<FileDescriptor>> wait_for_fds(
    std::vector<std::reference_wrapper<FileDescriptor>>& fds,
    std::chrono::milliseconds timeout) {

    if (fds.empty()) {
        return {};
    }

    FdSet read_fds{};
    for (auto& fd : fds) {
        read_fds.add(fd);
    }

    timeval tv{};
    tv.tv_sec  = std::chrono::duration_cast<std::chrono::seconds>(timeout).count();
    tv.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(
        timeout % std::chrono::seconds(1))
        .count();

    int ret = select(read_fds.max_fd() + 1, read_fds.native(), nullptr, nullptr, &tv);
    if (ret < 0) {
        throw std::runtime_error("select() failed");
    }

    std::vector<std::reference_wrapper<FileDescriptor>> ready;
    for (auto& fd : fds) {
        if (read_fds.contains(fd)) {
            ready.push_back(fd);
        }
    }

    return ready;
}
