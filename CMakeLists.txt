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
    coroutines/async_io/echo/EchoClient.cpp
    coroutines/async_io/echo/EchoServer.cpp
    common/pipe/Pipe.cpp
    coroutines/run_async_tasks_over_event_loop.cpp
    coroutines/run_echo_over_reactor.cpp
    coroutines/run_generator.cpp
    coroutines/run_async_tasks.cpp
    no-coroutines/async_io/echo/EchoClient.cpp
    no-coroutines/async_io/echo/EchoServer.cpp
    no-coroutines/run_echo_over_reactor.cpp
    no-coroutines/run_async_tasks_over_event_loop.cpp
    no-coroutines/run_generator.cpp
    no-coroutines/run_async_tasks.cpp
    no-coroutines/samples.h
    no-coroutines/async_tasks/AsyncTaskRunner.h
)

target_include_directories(fun_with_coroutines PRIVATE .)
