# Coroutine return types

The four simple return types live directly in this folder. `Task<T>` and its
helper `FinalAwaiter` live in the `task/` subfolder because `Task<T>` adds two
concepts beyond the basic structure: a custom final awaiter and an awaitable
interface. Each header contains a `promise_type hooks` comment block.

| File | Role |
|------|------|
| `NeverSuspendCoroutine.h` | `suspend_never` at both suspension points |
| `Generator.h` | `co_yield`, lazy pull sequence |
| `ResumableCoroutine.h` | `suspend_always` at both suspension points, void return |
| `FinalSuspendCoroutine.h` | `suspend_never` start, `suspend_always` end |
| `task/Task.h` | `co_return` value, lazy, awaitable (coroutine chaining) |
| `task/FinalAwaiter.h` | awaiter used by `Task`'s `final_suspend`; resumes the caller via symmetric transfer |

## `promise_type` hooks

`unhandled_exception`: `std::terminate` on all types.

| Method | NeverSuspendCoroutine | Generator | ResumableCoroutine | FinalSuspendCoroutine | Task\<T\> |
|--------|-----------------------|-----------|--------------------|----------------------|-----------|
| initial_suspend | suspend_never | suspend_always | suspend_always | suspend_never | suspend_always |
| final_suspend | suspend_never | suspend_always | suspend_always | suspend_always | FinalAwaiter\<promise\> |
| return_void | yes | yes | yes | yes | no |
| return_value | no | no | no | no | yes |
| yield_value | no | suspend_always | no | no | no |
