#include "EchoServer.h"
#include "common/async_io/LineHandler.h"
#include "common/async_io/LineTransport.h"
#include "no-coroutines/4. async_io/LineServer.h"

namespace no_coroutines {

EchoServer::EchoServer(Reactor &reactor, const int read_fd, const int write_fd)
    : _reactor(reactor), _read_fd(read_fd), _write_fd(write_fd) {}

void EchoServer::run() {
    const async_io::LineTransport transport{_reactor, _read_fd, _write_fd};
    LineServer(transport).run(async_io::echo_line_handler());
}

}
