#include "no-coroutines/generator/Generator.h"
#include <print>

namespace no_coroutines {

Generator createGenerator(int count) {
    return Generator(count);
}

void run_generator() {
    auto generator = createGenerator(10);
    while (generator.next()){
        int value = generator.get_current_value();
        std::print("{}\n", value);
    }
    while (generator.next()){
        int value = generator.get_current_value();
        std::print("{}\n", value);
    }
}

}
