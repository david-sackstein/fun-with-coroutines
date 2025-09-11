cmake_minimum_required(VERSION 3.31)
project(fun_with_coroutines)

set(CMAKE_CXX_STANDARD 20)

add_executable(fun_with_coroutines generator/run_generator.cpp
    main.cpp
    generator/run_generator.cpp
    async_tasks/EventLoop.cpp
    async_tasks/run_async_tasks.cpp)
