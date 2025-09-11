#include "coroutines/01-generator/Fibonacci.h"

namespace coroutines {

// Produce the first count Fibonacci numbers one at a time.
Generator fibonacci(const int count) {
    int a = 0;
    int b = 1;
    for (int i = 0; i < count; ++i) {
        // Return the current number.
        // Suspend until the caller asks for the next one.
        co_yield a;

        // Advance to the next pair in the sequence.
        const int next = a + b;
        a = b;
        b = next;
    }
}

}
