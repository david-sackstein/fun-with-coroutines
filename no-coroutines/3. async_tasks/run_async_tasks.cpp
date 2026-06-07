namespace no_coroutines {

void run_unmarshalled_sample();
void run_marshalled_sample();

void run_async_tasks() {
    run_unmarshalled_sample();
    run_marshalled_sample();
}

}
