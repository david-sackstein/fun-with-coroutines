#define COROUTINES

#ifdef COROUTINES
    #include "coroutines/samples.h"
#else
    #include "no-coroutines/samples.h"
#endif

int main() {
    run_generator();
    run_async_tasks_2();
    run_async_tasks_over_event_loop();
    run_echo_over_reactor();
}
