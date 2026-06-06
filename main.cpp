#define COROUTINES

#ifdef COROUTINES
    #include "coroutines/samples.h"
#else
    #include "no-coroutines/samples.h"
#endif

int main() {
    run_generator();                 // 1. generator
    // run_parser();                 // 2. parser (task 02)
    run_async_tasks();               // 3. async_tasks
    run_async_tasks_marshalled();    // 4. async_tasks_marshalled
    run_async_io();                  // 5. async_io
}
