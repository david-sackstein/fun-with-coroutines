#include "common/io/print.h"
#include "no-coroutines/1. generator/Generator.h"

namespace no_coroutines {

Generator fibonacci(const int count) {
    return Generator(count);
}

void run_generator() {
    io::print("Fibonacci (10 terms):\n");
    auto generator = fibonacci(10);
    for (const int value : generator) {
        io::print("{}\n", value);
    }

    // Second loop - generator is already exhausted, so nothing prints
    io::print("Second pass (exhausted):\n");
    for (const int value : generator) {
        io::print("{}\n", value);
    }
}

}
