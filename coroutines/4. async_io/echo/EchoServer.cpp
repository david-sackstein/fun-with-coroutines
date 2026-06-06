#include "common/async_io/LineHandler.h"
#include "common/async_io/LineTransport.h"
#include "coroutines/4. async_io/LineServer.h"
#include "coroutines/4. async_io/echo/EchoServer.h"

namespace coroutines {

EchoServer::EchoServer(Reactor &reactor, const int read_fd, const int write_fd)
    : _reactor(reactor), _read_fd(read_fd), _write_fd(write_fd) {}

AsyncIoCoroutine EchoServer::run() const {
    const async_io::LineTransport transport{_reactor, _read_fd, _write_fd};
    return run_line_server(transport, async_io::echo_line_handler());
}

}
