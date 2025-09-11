#include "no-coroutines/04-async-io/async/AsyncIo.h"

#include <cerrno>
#include <functional>
#include <memory>

#include <unistd.h>

namespace no_coroutines {

[[nodiscard]] static bool should_retry(const ssize_t n);

// Stop when a newline byte has been received or the buffer has no more room.
ReadStopWhen stop_at_newline_or_full() {
    return [](const char *buf, const size_t off, const size_t limit) {
        return (off > 0 && buf[off - 1] == '\n') || off >= limit;
    };
}

// Stop when the buffer holds exactly limit bytes.
ReadStopWhen stop_at_exact_bytes() {
    return [](const char *, const size_t off, const size_t limit) {
        return off >= limit;
    };
}

// Register a self-re-posting read handler that accumulates bytes until stop_when returns true.
void post_read(
    Reactor &reactor,
    const int fd,
    char *buffer,
    const size_t limit,
    ReadStopWhen stop_when,
    IoComplete on_complete)
{
    // offset is shared between iterations of the handler.
    auto offset = std::make_shared<size_t>(0);

    std::function<void(int)> read_handler;
    read_handler = [&reactor, fd, buffer, limit, offset,
                    stop_when = std::move(stop_when),
                    on_complete = std::move(on_complete),
                    read_handler](const int io_fd) mutable {
        const ssize_t n = ::read(io_fd, buffer + *offset, limit - *offset);

        if (should_retry(n)) {
            // The fd is not ready. Re-register and try again on the next wakeup.
            reactor.post(fd, Reactor::FdMode::Read, read_handler);
            return;
        }

        if (n <= 0) {
            // EOF or an unrecoverable error. Deliver however many bytes arrived so far.
            reactor.remove(fd, Reactor::FdMode::Read);
            on_complete(*offset);
            return;
        }

        *offset += static_cast<size_t>(n);

        if (!stop_when(buffer, *offset, limit)) {
            // Stop condition not met. Re-register to read more.
            reactor.post(fd, Reactor::FdMode::Read, read_handler);
        } else {
            // Stop condition satisfied. Deliver the accumulated bytes.
            reactor.remove(fd, Reactor::FdMode::Read);
            on_complete(*offset);
        }
    };

    reactor.post(fd, Reactor::FdMode::Read, read_handler);
}

// Register a self-re-posting write handler that sends all bytes from buffer.
void post_write(
    Reactor &reactor,
    const int fd,
    const char *buffer,
    const size_t total,
    IoComplete on_complete)
{
    // offset is shared between iterations of the handler.
    auto offset = std::make_shared<size_t>(0);

    std::function<void(int)> write_handler;
    write_handler = [&reactor, fd, buffer, total, offset,
                     on_complete = std::move(on_complete),
                     write_handler](const int io_fd) mutable {
        const ssize_t n = ::write(io_fd, buffer + *offset, total - *offset);

        if (should_retry(n)) {
            // The fd is not ready. Re-register and try again on the next wakeup.
            reactor.post(fd, Reactor::FdMode::Write, write_handler);
            return;
        }

        if (n <= 0) {
            // Write error. Deliver however many bytes were sent so far.
            reactor.remove(fd, Reactor::FdMode::Write);
            on_complete(*offset);
            return;
        }

        *offset += static_cast<size_t>(n);

        if (*offset < total) {
            // More bytes remain. Re-register to write the rest.
            reactor.post(fd, Reactor::FdMode::Write, write_handler);
        } else {
            // All bytes written. Deliver the total.
            reactor.remove(fd, Reactor::FdMode::Write);
            on_complete(*offset);
        }
    };

    reactor.post(fd, Reactor::FdMode::Write, write_handler);
}

// Return true for errors that mean "try again later" (non-blocking fd not ready, or signal).
[[nodiscard]] static bool should_retry(const ssize_t n) {
    return n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
}

}
