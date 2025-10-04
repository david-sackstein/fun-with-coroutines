cmake_minimum_required(VERSION 3.31)
project(fun_with_coroutines)

set(CMAKE_CXX_STANDARD 20)

# Enable AddressSanitizer
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    link_libraries(-fsanitize=address)
endif()

add_executable(fun_with_coroutines
    main.cpp
    generator/run_generator.cpp
    async_tasks/EventLoop.cpp
    async_tasks/run_async_tasks.cpp
    async_io/run_async_io.cpp
    async_io/run_echo_demo.cpp
    async_io/reactor/Reactor.cpp
    async_io/echo/Pipe.cpp
    async_io/echo/EchoClient.cpp
    async_io/echo/EchoServer.cpp
)

target_include_directories(fun_with_coroutines PRIVATE
    async_io
)
