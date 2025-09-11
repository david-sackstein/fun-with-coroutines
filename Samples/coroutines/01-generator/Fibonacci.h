#pragma once

// Coroutine generator that produces the Fibonacci sequence one value at a time.
// Each co_yield suspends until the caller calls next().

#include "coroutines/return_types/Generator.h"

namespace coroutines {

// Return a Generator that yields the first count Fibonacci numbers.
Generator fibonacci(int count);

}
