#pragma once

#include <array>

class Pipe {
public:
    Pipe();
    ~Pipe();
    
    Pipe(const Pipe&) = delete;
    Pipe& operator=(const Pipe&) = delete;
    
    int read_fd() const { return _fds[0]; }
    int write_fd() const { return _fds[1]; }

private:
    std::array<int, 2> _fds;
};

