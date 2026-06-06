#pragma once

#include "common/async_io/LineTransport.h"
#include "coroutines/4. async_io/async/AsyncIoCoroutine.h"

namespace coroutines {

[[nodiscard]] AsyncIoCoroutine line_rpc(const async_io::LineTransport &transport, async_io::LineRpcCall &call);

}
