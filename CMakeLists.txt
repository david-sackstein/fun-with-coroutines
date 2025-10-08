cmake_minimum_required(VERSION 3.31)
project(fun_with_coroutines)

set(CMAKE_CXX_STANDARD 23)

# Enable AddressSanitizer
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    link_libraries(-fsanitize=address)
endif()

add_executable(fun_with_coroutines
    main.cpp
    common/reactor/Reactor.cpp
    common/event_loop/EventLoop.cpp
    "coroutines/4. async_io/echo/EchoClient.cpp"
    "coroutines/4. async_io/echo/EchoServer.cpp"
    common/pipe/Pipe.cpp
    "coroutines/3. async_tasks_marshalled/run_async_tasks_marshalled.cpp"
    "coroutines/4. async_io/run_async_io.cpp"
    "coroutines/1. generator/run_generator.cpp"
    "coroutines/2. async_tasks/run_async_tasks.cpp"
    "no-coroutines/4. async_io/echo/EchoClient.cpp"
    "no-coroutines/4. async_io/echo/EchoServer.cpp"
    "no-coroutines/4. async_io/run_async_io.cpp"
    "no-coroutines/3. async_tasks_marshalled/run_async_tasks_marshalled.cpp"
    "no-coroutines/1. generator/run_generator.cpp"
    "no-coroutines/2. async_tasks/run_async_tasks.cpp"
    no-coroutines/samples.h
    "no-coroutines/3. async_tasks_marshalled/AsyncTaskRunnerMarshalled.h"
)

target_include_directories(fun_with_coroutines PRIVATE .)
