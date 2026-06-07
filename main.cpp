#define COROUTINES

#ifdef COROUTINES
    #include "coroutines/samples.h"
#else
    #include "no-coroutines/samples.h"
#endif

int main() {
    run_generator();                 // 1. generator
    run_parser();                    // 2. parser
    run_async_tasks();               // 3. async_tasks
    run_async_io();                  // 4. async_io
    run_aggregation();               // 5. aggregation
}
