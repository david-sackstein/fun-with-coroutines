#pragma once

#include "Fd.h"
#include <sys/select.h>

class FdSet {
    fd_set _fds;
    int _max_fd = -1;

public:
    FdSet(std::vector<std::reference_wrapper<Fd>>& fds) {
        FD_ZERO(&_fds);
        add(fds);
    }

    bool contains(const Fd& fd) const {
        return FD_ISSET(fd.get(), &_fds);
    }

    int max_fd() const {
        return _max_fd;
    }

    fd_set* native() {
        return &_fds;
    }

private:

    void add(std::vector<std::reference_wrapper<Fd>>& fds){
        for (auto& fd : fds) {
            add(fd);
        }
    }

    void add(const Fd& fd) {
        int raw_fd = fd.get();
        if (raw_fd >= FD_SETSIZE) {
            throw std::runtime_error("File descriptor exceeds FD_SETSIZE");
        }
        FD_SET(raw_fd, &_fds);
        if (raw_fd > _max_fd) {
            _max_fd = raw_fd;
        }
    }
};