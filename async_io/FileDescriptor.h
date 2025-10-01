#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <memory>
#include <iostream>

static const int invalid_fd = -1;

class FileDescriptor {
    int _fd;

public:
    explicit FileDescriptor(int fd) : _fd(fd) {
        if (_fd <= invalid_fd) {
            throw std::runtime_error("Invalid file descriptor");
        }
    }

    ~FileDescriptor() {
        if (_fd != invalid_fd) {
            close(_fd);
        }
    }

    // non-copyable
    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    // non-movable
    FileDescriptor(FileDescriptor&& other) noexcept = delete;
    FileDescriptor& operator=(FileDescriptor&& other) noexcept = delete;

    int get() const { return _fd; }
};
