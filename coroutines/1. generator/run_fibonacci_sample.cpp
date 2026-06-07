#include "common/io/print.h"
#include "coroutines/1. generator/Generator.h"

namespace coroutines {

Generator fibonacci(const int count) {
    int a = 0;
    int b = 1;
    for (int i = 0; i < count; ++i) {
        co_yield a;
        const int next = a + b;
        a = b;
        b = next;
    }
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
