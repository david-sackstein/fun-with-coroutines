#include "common/io/print.h"
#include "no-coroutines/1. generator/Generator.h"

namespace no_coroutines {

Generator fibonacci(const int count) {
    return Generator(count);
}

void run_fibonacci_sample() {
    io::print("Fibonacci (10 terms):\n");
    auto generator = fibonacci(10);
    for (const int value : generator) {
        io::print("{}\n", value);
    }

    io::print("Second pass (exhausted):\n");
    for (const int value : generator) {
        io::print("{}\n", value);
    }
}

}
