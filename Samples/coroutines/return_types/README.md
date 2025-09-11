# Coroutine return types

All headers in this folder. Each header has a `promise_type hooks` comment block aligned with the table below.

| File | Role |
|------|------|
| `NeverSuspendCoroutine.h` | `suspend_never` at start and end |
| `Generator.h` | `co_yield`, lazy |
| `Task.h` | `co_return` value, lazy |
| `ResumableCoroutine.h` | `suspend_always` at start and end, void return |
| `FinalSuspendCoroutine.h` | `suspend_never` start, `suspend_always` end |

## `promise_type` hooks

`unhandled_exception`: `std::terminate` on all types.

| Method | NeverSuspendCoroutine | Generator | Task\<T\> | ResumableCoroutine | FinalSuspendCoroutine |
|--------|----------------------|-----------|----------|-------------------|----------------------|
| initial_suspend | suspend_never | suspend_always | suspend_always | suspend_always | suspend_never |
| final_suspend | suspend_never | suspend_always | suspend_always | suspend_always | suspend_always |
| return_void | yes | yes | no | yes | yes |
| return_value | no | no | yes | no | no |
| yield_value | no | suspend_always | no | no | no |
