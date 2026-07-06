#include "coroutines/05-task-composition/TaskChain.h"

namespace coroutines {

Task<int> inner() {
    co_return 10;
}

// co_await suspends middle() until inner() completes, then resumes with inner()'s result.
Task<int> middle() {
    const int value = co_await inner();
    co_return value + 20;
}

// co_await suspends outer() until middle() completes, then resumes with middle()'s result.
Task<int> outer() {
    const int value = co_await middle();
    co_return value + 30;
}

int run_task_chain() {
    return sync_wait(outer());
}

}
