#include "Selector.h"

#include <system_error>
#include <cerrno>
#include <unistd.h>

Selector::Selector(const std::vector<FdHandler>& handlers) :
    _handlers(handlers),
    _notify()
{}

void Selector::run() {
    if (_handlers.empty()) {
        return;
    }

    while (_running) {
        FdSet fdSet(with_wakeup_fds());
        wait_once(fdSet);
        dispatch_ready(fdSet);
    }
}

void Selector::stop() noexcept {
    _running = false;
    _notify.notify();
}

void Selector::wait_once(FdSet& fdSet) {
    while (true) {
        int ret = ::select(fdSet.max_fd() + 1, fdSet.native(), nullptr, nullptr, nullptr);
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


void Selector::dispatch_ready(const FdSet& fdSet) {
    for (const auto& handler : _handlers) {
        if (fdSet.contains(handler.fd)) {
            handler.handler(handler.fd);
        }
    }
}

auto Selector::with_wakeup_fds() -> std::vector<int> {
    std::vector<int> fds;
    for (const auto& handler : _handlers) {
        fds.push_back(handler.fd);
    }
    fds.push_back(_notify.arm());
    return fds;
}