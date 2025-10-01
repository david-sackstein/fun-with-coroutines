#include "Selector.h"

#include <system_error>
#include <cerrno>
#include <unistd.h>
#include <chrono>

using std::chrono::milliseconds;

Selector::Selector(FdVector &fds) :
    _fds(fds),
    _notify()
{}

auto Selector::wait_for_fds() -> FdVector {
    if (_fds.empty()) {
        return {};
    }

    FdVector with_wakeup = _fds;
    with_wakeup.push_back(std::ref(_notify.read_end()));
    FdSet fdSet(with_wakeup);

    wait_once(fdSet);

    if (fdSet.contains(_notify.read_end())) {
        _notify.drain();
    }

    return get_ready_fds(fdSet);
}

void Selector::add_work() {
    std::lock_guard<std::mutex> lg(mtx);
    ++outstanding_work;
}

void Selector::remove_work() {
    std::lock_guard<std::mutex> lg(mtx);
    if (outstanding_work > 0) {
        --outstanding_work;
        if (outstanding_work == 0) {
            interrupt_wait();
        }
    }
}

void Selector::interrupt_wait() {
    _notify.notify();
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
        int ret = ::select(fdSet.max_fd() + 1, fdSet.native(), nullptr, nullptr, nullptr);
        if (ret < 0 && errno == EINTR) {
            // Rebuild fd_set and retry
            FdVector with_wakeup = _fds;
            with_wakeup.push_back(std::ref(_notify.read_end()));
            fdSet = FdSet(with_wakeup);
            continue;
        }
        if (ret < 0) {
            throw std::system_error(errno, std::generic_category(), "select() failed");
        }
        break;
    }
}