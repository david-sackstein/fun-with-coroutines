#include "coroutines/4. async_io/CalculatorRepl.h"
#include "no-coroutines/4. async_io/CalculatorRepl.h"

#include "tests/Side.h"

#include <gtest/gtest.h>

#include <iostream>
#include <string_view>

namespace {

[[nodiscard]] bool is_gtest_flag(const std::string_view arg) {
    return arg.starts_with("--gtest_");
}

[[nodiscard]] bool should_run_tests(const int argc, char **argv) {
    if (argc < 2) {
        return true;
    }

    if (is_gtest_flag(argv[1])) {
        return true;
    }

    for (int index = 2; index < argc; ++index) {
        if (is_gtest_flag(argv[index])) {
            return true;
        }
    }

    return false;
}

Side parse_repl_side(const int argc, char **argv) {
    if (argc >= 2 && std::string_view{argv[1]} == "--no-coroutines") {
        return Side::NoCoroutines;
    }
    return Side::Coroutines;
}

void run_repl(const Side side) {
    if (side == Side::Coroutines) {
        coroutines::run_calculator_repl();
        return;
    }
    no_coroutines::run_calculator_repl();
}

}

int main(int argc, char **argv) {
    if (should_run_tests(argc, argv)) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    run_repl(parse_repl_side(argc, argv));
    return 0;
}
