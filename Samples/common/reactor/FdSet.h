#pragma once

// POSIX fd_set wrapper for use with select().
// Constructed from a list of file descriptors. Tracks the maximum fd value
// required by the first argument to select(nfds, ...).

#include <sys/select.h>

#include <stdexcept>
#include <vector>

class FdSet {
public:
    explicit FdSet(const std::vector<int>& fds) {
        FD_ZERO(&_fds);
        add(fds);
    }

    // Return true if fd was marked ready by select().
    [[nodiscard]] bool contains(int fd) const {
        return FD_ISSET(fd, &_fds);
    }

    // Return the highest fd in the set; needed for the nfds argument of select().
    [[nodiscard]] int max_fd() const {
        return _max_fd;
    }

    // Return a pointer to the raw fd_set for passing to select().
    fd_set* native() {
        return &_fds;
    }

private:
    // Add each fd in the list.
    void add(const std::vector<int>& fds) {
        for (const int fd : fds) {
            add(fd);
        }
    }

    // Add a single fd and update the maximum.
    void add(int fd) {
        if (fd >= FD_SETSIZE) {
            throw std::runtime_error("File descriptor exceeds FD_SETSIZE");
        }
        FD_SET(fd, &_fds);
        if (fd > _max_fd) {
            _max_fd = fd;
        }
    }

    fd_set _fds{};
    int _max_fd = -1;
};
