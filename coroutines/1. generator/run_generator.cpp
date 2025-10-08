#include "coroutines/1. generator/Generator.h"

namespace coroutines {

Generator createGenerator(int count) {
    for (int i=0; i<count; i++) {
        co_yield i;
    }
}

void run_generator() {
    // Using range-based for loop with iterator
    auto generator = createGenerator(10);
    for (int value : generator) {
        std::print("{}\n", value);
    }
    
    // Second loop - generator is already exhausted, so nothing prints
    for (int value : generator) {
        std::print("{}\n", value);
    }
}

}
