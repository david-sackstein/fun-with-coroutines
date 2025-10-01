#pragma once

#include <system_error>
#include <cerrno>
#include <unistd.h>

struct PipeFds {
    int r;
    int w;

    PipeFds() {
        int fds[2];
        if (::pipe(fds) != 0) {
            throw std::system_error(errno, std::generic_category(), "pipe() failed");
        }
        r = fds[0];
        w = fds[1];
    }
};

