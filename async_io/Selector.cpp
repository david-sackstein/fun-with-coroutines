#include "Selector.h"

#include <system_error>
#include <cerrno>
#include <unistd.h>
#include <chrono>

using std::chrono::milliseconds;

Selector::Selector(FdVector &fds) :
    _fds(fds)
{
    int pipe_fd[2];
    if (::pipe(pipe_fd) != 0) {
        throw std::system_error(errno, std::generic_category(), "pipe() failed");
    }
    _wakeup_read = std::make_unique<Fd>(pipe_fd[0]);
    _wakeup_write = std::make_unique<Fd>(pipe_fd[1]);
}

auto Selector::wait_for_fds() -> FdVector {
    if (_fds.empty()) {
        return {};
    }

    // Build local fd_set each wait, including wakeup read end
    FdVector with_wakeup = _fds;
    with_wakeup.push_back(std::ref(*_wakeup_read));
    FdSet fdSet(with_wakeup);

    wait_once(fdSet);

    // Drain wakeup if it fired
    if (fdSet.contains(*_wakeup_read)) {
        char buf[64];
        for (;;) {
            ssize_t n = ::read(_wakeup_read->get(), buf, sizeof(buf));
            if (n > 0) break;
            if (n < 0 && errno == EINTR) continue;
            break;
        }
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
    if (!_wakeup_write) return;
    char b = 1;
    for (;;) {
        ssize_t n = ::write(_wakeup_write->get(), &b, 1);
        if (n >= 0) break;
        if (errno == EINTR) continue;
        break;
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

void Selector::wait_once(FdSet& fdSet) {
    for (;;) {
        int ret = ::select(fdSet.max_fd() + 1, fdSet.native(), nullptr, nullptr, nullptr);
        if (ret < 0 && errno == EINTR) {
            // Rebuild fd_set and retry
            FdVector with_wakeup = _fds;
            with_wakeup.push_back(std::ref(*_wakeup_read));
            fdSet = FdSet(with_wakeup);
            continue;
        }
        if (ret < 0) {
            throw std::system_error(errno, std::generic_category(), "select() failed");
        }
        break;
    }
}