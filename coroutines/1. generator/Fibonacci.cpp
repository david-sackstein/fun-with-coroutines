#include "coroutines/1. generator/Fibonacci.h"

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

}
