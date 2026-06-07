cmake_minimum_required(VERSION 3.31)

project(fun_with_coroutines LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    common/*.cpp
    coroutines/*.cpp
    no-coroutines/*.cpp
)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    "tests/*/*/*.cpp"
)

add_executable(fun_with_coroutines
    main.cpp
    ${SOURCES}
    ${TEST_SOURCES}
)

target_compile_features(fun_with_coroutines PRIVATE cxx_std_20)

set_target_properties(fun_with_coroutines PROPERTIES
    CXX_EXTENSIONS OFF
)

target_compile_definitions(fun_with_coroutines PRIVATE FWC_TESTING)

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

target_link_libraries(fun_with_coroutines PRIVATE
    GTest::gtest
)

include(GoogleTest)
gtest_discover_tests(fun_with_coroutines)

enable_testing()
add_test(NAME unit COMMAND fun_with_coroutines)
