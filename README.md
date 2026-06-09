# Fun with coroutines

Side-by-side C++ demos: coroutines vs callbacks/manual code. Five lessons, one binary.

## Quick start

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build          # or: ./build/fun_with_coroutines
```

CLion uses `cmake-build-debug/` by default (gitignored).

### Run

| Command | What |
|---------|------|
| `./fun_with_coroutines` | All GTest cases (20 tests) |
| `./fun_with_coroutines --gtest_filter=*Demo04*` | Demo 4 tests (both sides) |
| `./fun_with_coroutines demo` | Live calculator REPL (coroutines) |
| `./fun_with_coroutines --no-coroutines` | Live calculator REPL (callbacks) |

Demos 1–3 and 5 are exercised only through tests. Demo 4 is the only live REPL.

`FWC_TESTING` is defined for the test binary (short async delays). The REPL uses longer safety timeouts.

## Repository layout

```
common/                    Shared infrastructure (reactor, event loop, pipes, parser script, …)
coroutines/
  return_types/            Four coroutine return types — see README there
  01-generator/ … 05-aggregation/   Lesson code (co_await / co_yield / co_return)
no-coroutines/
  01-generator/ … 05-aggregation/   Same lessons, callbacks / manual state
tests/
  01-generator/coroutines/     GTest harness (coroutine side)
  01-generator/no-coroutines/    GTest harness (callback side)
  … (same for demos 02–05)
main.cpp                   Tests by default; REPL when args are present
```

### How to study a demo

1. Read [`coroutines/return_types/README.md`](coroutines/return_types/README.md) if the demo uses a coroutine return type.
2. Open the matching `NN-name/` folders under `coroutines/` and `no-coroutines/`.
3. Diff the **same filenames** in each tree (see table below).
4. Run tests: `--gtest_filter=*Demo03*` (matches both sides).

## Demos

| # | Folder | Lesson |
|---|--------|--------|
| 1 | `01-generator` | Lazy sequences (`co_yield` vs manual generator class) |
| 2 | `02-parser` | Line protocol / streaming parser |
| 3 | `03-async-tasks` | Unmarshalled vs marshalled async (`co_await` vs nested callbacks) |
| 4 | `04-async-io` | Calculator over pipes (async I/O vs reactor callbacks) |
| 5 | `05-aggregation` | `co_return` with values (`Task<T>`) |

## File pairing (coroutines ↔ no-coroutines)

Same demo, same filename; only the parent tree differs.

| Demo | Paired files (both sides) |
|------|---------------------------|
| 1 | `Fibonacci.h`, `TreeTraversal.h` (+ `Fibonacci.cpp`, `TreeTraversal.cpp` on coroutines side) |
| 2 | `ProtocolParser.h`, `ProtocolParser.cpp` |
| 3 | `AsyncTask.h`, `AsyncTaskMarshalled.h`, `UnmarshalledTask.*`, `MarshalledTask.*` |
| 4 | `async/AsyncIo.h` (+ `AsyncBuffer.h` coroutines-only), `calc/CalcClient.*`, `calc/CalcServer.*` |
| 5 | `ParserStore.*`, `AsyncAggregate.*`, `CalculatorBatch.*` |

Only the **coroutines** tree has `return_types/` (promise wrappers). The callback side uses ordinary functions and classes.

### Test harness

Each demo has `tests/NN-name/coroutines/` and `tests/NN-name/no-coroutines/` with the same basename (`Generator.h`, `Parser.h`, …). Fixture classes are `Demo01GeneratorCoroutines` / `Demo01GeneratorNoCoroutines`, etc. — the `Coroutines` / `NoCoroutines` suffix is required so both sides can live in one binary.

## Requirements

- C++20 compiler with coroutine support (GCC 13+, Clang 16+)
- CMake 3.31+
