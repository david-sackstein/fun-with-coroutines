#include "no-coroutines/04-async-io/async/AsyncIo.h"

#include <cerrno>
#include <functional>
#include <memory>

#include <unistd.h>

namespace no_coroutines {

namespace {

[[nodiscard]] bool should_retry(const ssize_t n) {
    return n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
}

}

ReadStopWhen stop_at_newline_or_full() {
    return [](const char *buf, const size_t off, const size_t limit) {
        return (off > 0 && buf[off - 1] == '\n') || off >= limit;
    };
}

ReadStopWhen stop_at_exact_bytes() {
    return [](const char *, const size_t off, const size_t limit) {
        return off >= limit;
    };
}

void post_read(
    Reactor &reactor,
    const int fd,
    char *buffer,
    const size_t limit,
    ReadStopWhen stop_when,
    IoComplete on_complete)
{
    auto offset = std::make_shared<size_t>(0);

    std::function<void(int)> read_handler;
    read_handler = [&reactor, fd, buffer, limit, offset,
                    stop_when = std::move(stop_when),
                    on_complete = std::move(on_complete),
                    read_handler](const int io_fd) mutable {
        const ssize_t n = ::read(io_fd, buffer + *offset, limit - *offset);

        if (should_retry(n)) {
            // Retry
            reactor.post(fd, Reactor::FdMode::Read, read_handler);
            return;
        }

        if (n <= 0) {
            // EOF or error
            reactor.remove(fd, Reactor::FdMode::Read);
            on_complete(*offset);
            return;
        }

        *offset += static_cast<size_t>(n);

        if (!stop_when(buffer, *offset, limit)) {
            // Need more data
            reactor.post(fd, Reactor::FdMode::Read, read_handler);
        } else {
            // Done
            reactor.remove(fd, Reactor::FdMode::Read);
            on_complete(*offset);
        }
    };

    reactor.post(fd, Reactor::FdMode::Read, read_handler);
}

void post_write(
    Reactor &reactor,
    const int fd,
    const char *buffer,
    const size_t total,
    IoComplete on_complete)
{
    auto offset = std::make_shared<size_t>(0);

    std::function<void(int)> write_handler;
    write_handler = [&reactor, fd, buffer, total, offset,
                     on_complete = std::move(on_complete),
                     write_handler](const int io_fd) mutable {
        const ssize_t n = ::write(io_fd, buffer + *offset, total - *offset);

        if (should_retry(n)) {
            // Retry
            reactor.post(fd, Reactor::FdMode::Write, write_handler);
            return;
        }

        if (n <= 0) {
            // Error
            reactor.remove(fd, Reactor::FdMode::Write);
            on_complete(*offset);
            return;
        }

        *offset += static_cast<size_t>(n);

        if (*offset < total) {
            // Need to write more
            reactor.post(fd, Reactor::FdMode::Write, write_handler);
        } else {
            // Done
            reactor.remove(fd, Reactor::FdMode::Write);
            on_complete(*offset);
        }
    };

    reactor.post(fd, Reactor::FdMode::Write, write_handler);
}

}
