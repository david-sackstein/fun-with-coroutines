#include "generator/Generator.h"

Generator createGenerator(int count) {
    for (int i=0; i<count; i++) {
        co_yield i;
    }
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
