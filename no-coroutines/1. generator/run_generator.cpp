#include "no-coroutines/1. generator/Generator.h"

#include <print>

namespace no_coroutines {

Generator createGenerator(int count) {
    return Generator(count);
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
