#pragma once

#include "FileDescriptor.h"
#include <sys/select.h>

class FdSet {
    fd_set _fds;
    int _max_fd = -1;

public:
    FdSet() {
        FD_ZERO(&_fds);
    }

    void add(const FileDescriptor& fd) {
        int raw_fd = fd.get();
        if (raw_fd >= FD_SETSIZE) {
            throw std::runtime_error("File descriptor exceeds FD_SETSIZE");
        }
        FD_SET(raw_fd, &_fds);
        if (raw_fd > _max_fd) {
            _max_fd = raw_fd;
        }
    }

    bool contains(const FileDescriptor& fd) const {
        return FD_ISSET(fd.get(), &_fds);
    }

    int max_fd() const {
        return _max_fd;
    }

    fd_set* native() {
        return &_fds;
    }
};