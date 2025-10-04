#include "Pipe.h"
#include <unistd.h>
#include <system_error>
#include <cerrno>

Pipe::Pipe() {
    if (::pipe(_fds.data()) < 0) {
        throw std::system_error(errno, std::generic_category(), "pipe() failed");
    }
}

Pipe::~Pipe() {
    ::close(_fds[0]);
    ::close(_fds[1]);
}

