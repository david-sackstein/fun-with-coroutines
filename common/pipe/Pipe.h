#pragma once

#include <array>

class Pipe {
public:
    Pipe();
    ~Pipe();
    
    Pipe(const Pipe&) = delete;
    Pipe& operator=(const Pipe&) = delete;
    
    [[nodiscard]] int read_fd() const { return _fds[0]; }
    [[nodiscard]] int write_fd() const { return _fds[1]; }

private:
    std::array<int, 2> _fds{};
};
