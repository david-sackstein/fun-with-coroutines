#pragma once

#include <sys/select.h>
#include <vector>
#include <stdexcept>

class FdSet {
    fd_set _fds;
    int _max_fd = -1;

public:
    FdSet(const std::vector<int>& fds) {
        FD_ZERO(&_fds);
        add(fds);
    }

    bool contains(int fd) const {
        return FD_ISSET(fd, &_fds);
    }

    int max_fd() const {
        return _max_fd;
    }

    fd_set* native() {
        return &_fds;
    }

private:

    void add(const std::vector<int>& fds){
        for (int fd : fds) {
            add(fd);
        }
    }

    void add(int fd) {
        if (fd >= FD_SETSIZE) {
            throw std::runtime_error("File descriptor exceeds FD_SETSIZE");
        }
        FD_SET(fd, &_fds);
        if (fd > _max_fd) {
            _max_fd = fd;
        }
    }
};