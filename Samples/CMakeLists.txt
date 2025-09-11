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

# repl_main.cpp defines main() and must not be linked into the gtest binary.
list(FILTER SOURCES EXCLUDE REGEX "repl_main\\.cpp$")

if(WIN32)
    list(FILTER SOURCES EXCLUDE REGEX
        "common/reactor|common/pipe|common/async_io|04-async-io|05-aggregation")
    list(FILTER TEST_SOURCES EXCLUDE REGEX
        "04-async-io|05-aggregation")
endif()

# CLion gutter / Google Test integration expects a dedicated *_test target linked with gtest.
# OUTPUT_NAME keeps the on-disk binary as fun_with_coroutines.
add_executable(fun_with_coroutines_test
    ${SOURCES}
    ${TEST_SOURCES}
)

target_compile_features(fun_with_coroutines_test PRIVATE cxx_std_20)

set_target_properties(fun_with_coroutines_test PROPERTIES
    CXX_EXTENSIONS OFF
    OUTPUT_NAME fun_with_coroutines
)

target_compile_options(fun_with_coroutines_test PRIVATE
    -Werror
    $<$<NOT:$<BOOL:${WIN32}>>:-fsanitize=address -fno-omit-frame-pointer>
)

target_link_options(fun_with_coroutines_test PRIVATE
    $<$<NOT:$<BOOL:${WIN32}>>:-fsanitize=address>
)

target_include_directories(fun_with_coroutines_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fun_with_coroutines_test PRIVATE
    GTest::gtest_main
)

if(NOT WIN32)
    # Interactive calculator REPL — run this target to type expressions manually.
    add_executable(calc_repl
        coroutines/04-async-io/repl_main.cpp
        coroutines/04-async-io/run_async_io.cpp
        coroutines/04-async-io/calc/CalcServer.cpp
        coroutines/04-async-io/calc/CalcClient.cpp
        common/event_loop/EventLoop.cpp
        common/parser/InputBuffer.cpp
        common/pipe/Pipe.cpp
        common/reactor/Reactor.cpp
    )

    target_compile_features(calc_repl PRIVATE cxx_std_20)

    set_target_properties(calc_repl PROPERTIES CXX_EXTENSIONS OFF)

    target_compile_options(calc_repl PRIVATE
        -Werror
        -fsanitize=address
        -fno-omit-frame-pointer
    )

    target_link_options(calc_repl PRIVATE
        -fsanitize=address
    )

    target_include_directories(calc_repl PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

enable_testing()

include(GoogleTest)
gtest_discover_tests(fun_with_coroutines_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DISCOVERY_MODE POST_BUILD
)
