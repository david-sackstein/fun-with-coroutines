#include "Pipe.h"

#include <unistd.h>
#include <fcntl.h>
#include <system_error>

// Create the pipe and set both ends to non-blocking mode.
Pipe::Pipe() {
    if (::pipe(_fds.data()) < 0) {
        throw std::system_error(errno, std::generic_category(), "pipe() failed");
    }
    fcntl(_fds[0], F_SETFL, O_NONBLOCK);
    fcntl(_fds[1], F_SETFL, O_NONBLOCK);
}

// Close both ends of the pipe.
Pipe::~Pipe() {
    ::close(_fds[0]);
    ::close(_fds[1]);
}
