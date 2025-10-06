#include "Pipe.h"

#include <unistd.h>
#include <fcntl.h>
#include <system_error>

Pipe::Pipe() {
    if (::pipe(_fds.data()) < 0) {
        throw std::system_error(errno, std::generic_category(), "pipe() failed");
    }

    // Set both ends to non-blocking mode
    fcntl(_fds[0], F_SETFL, O_NONBLOCK);
    fcntl(_fds[1], F_SETFL, O_NONBLOCK);
}

Pipe::~Pipe() {
    ::close(_fds[0]);
    ::close(_fds[1]);
}
