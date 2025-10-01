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
    async_tasks/unlock_guard.h
    async_tasks/work_guard.h
    async_io/run_async_io.cpp
    async_io/wait_for_fds.cpp
    async_io/wait_for_fds.h
    async_io/FileDescriptor.h
    async_io/FdSet.h
)
