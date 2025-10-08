#define COROUTINES

#ifdef COROUTINES
    #include "coroutines/samples.h"
#else
    #include "no-coroutines/samples.h"
#endif

int main() {
    run_generator();
    run_async_tasks();
    run_async_tasks_marshalled();
    run_async_io();
}
