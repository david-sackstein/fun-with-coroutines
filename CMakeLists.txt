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
    run_generator.cpp
    async_tasks/event_loop/EventLoop.cpp
    run_async_tasks.cpp
    run_echo_demo.cpp
    async_io/reactor/Reactor.cpp
    async_io/echo/Pipe.cpp
    async_io/echo/EchoClient.cpp
    async_io/echo/EchoServer.cpp
)

target_include_directories(fun_with_coroutines PRIVATE .)
