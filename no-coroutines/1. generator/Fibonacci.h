#pragma once

#include "no-coroutines/1. generator/Generator.h"

namespace no_coroutines {

inline Generator make_fibonacci(const int count) {
    return Generator(count);
}

}
