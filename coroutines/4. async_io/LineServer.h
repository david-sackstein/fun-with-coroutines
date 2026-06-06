#pragma once

#include "common/async_io/LineHandler.h"
#include "common/async_io/LineTransport.h"
#include "coroutines/4. async_io/async/AsyncIoCoroutine.h"

namespace coroutines {

[[nodiscard]] AsyncIoCoroutine run_line_server(const async_io::LineTransport &transport,
                                               async_io::LineHandler handler);

}
