#pragma once

// promise_type hooks — full comparison (unhandled_exception: std::terminate on all)
//
// Method             CoroutineObject   Generator         Task<T>           AsyncIoCoroutine
// initial_suspend    suspend_never     suspend_always    suspend_always    suspend_never
// final_suspend      suspend_never     suspend_always    suspend_always    suspend_always
// return_void        yes               yes               —                 yes
// return_value       —                 —                 yes               —
// yield_value        —                 suspend_always    —                 —
