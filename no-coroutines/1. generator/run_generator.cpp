#include "no-coroutines/1. generator/Generator.h"

#include <print>

namespace no_coroutines {

Generator fibonacci(const int count) {
    return Generator(count);
}

void run_generator() {
    std::print("Fibonacci (10 terms):\n");
    auto generator = fibonacci(10);
    for (const int value : generator) {
        std::print("{}\n", value);
    }

    // Second loop - generator is already exhausted, so nothing prints
    std::print("Second pass (exhausted):\n");
    for (const int value : generator) {
        std::print("{}\n", value);
    }
}

}
