#pragma once

// RAII wrapper for a non-blocking POSIX pipe.
// Both ends are set to O_NONBLOCK on construction and closed on destruction.

#include <array>

class Pipe {
public:
    Pipe();
    ~Pipe();

    Pipe(const Pipe&) = delete;
    Pipe& operator=(const Pipe&) = delete;

    // Return the read end of the pipe (registered with the Reactor for incoming data).
    [[nodiscard]] int read_fd() const {
        return _fds[0];
    }
    // Return the write end of the pipe (used to send data to the other side).
    [[nodiscard]] int write_fd() const {
        return _fds[1];
    }

private:
    std::array<int, 2> _fds{};
};
