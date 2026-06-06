cmake_minimum_required(VERSION 3.31)

project(fun_with_coroutines LANGUAGES CXX)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    common/*.cpp
    coroutines/*.cpp
    no-coroutines/*.cpp
)

add_executable(fun_with_coroutines
    main.cpp
    ${SOURCES}
)

target_compile_features(fun_with_coroutines PRIVATE cxx_std_20)

set_target_properties(fun_with_coroutines PROPERTIES
    CXX_EXTENSIONS OFF
)

target_compile_options(fun_with_coroutines PRIVATE
    -Werror
    -fsanitize=address
    -fno-omit-frame-pointer
)

target_link_options(fun_with_coroutines PRIVATE
    -fsanitize=address
)

target_include_directories(fun_with_coroutines PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)