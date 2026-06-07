#include "common/io/print.h"

namespace coroutines {

void run_parser_sample();
void run_async_sample();
void run_calculator_sample();

void run_aggregation() {
    io::print("=== Aggregation demo ===\n");
    io::print("Three samples — parser map, async aggregate, calculator RPC batch\n");

    run_parser_sample();
    run_async_sample();
    run_calculator_sample();

    io::print("\n=== Aggregation demo finished ===\n");
}

}
