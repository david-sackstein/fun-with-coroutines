#include "coroutines/04-async-io/CalculatorRepl.h"
#include "no-coroutines/04-async-io/CalculatorRepl.h"

#include <gtest/gtest.h>

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

}

int main(int argc, char **argv) {
    if (should_run_tests(argc, argv)) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    if (argc >= 2 && std::string_view{argv[1]} == "--no-coroutines") {
        no_coroutines::run_calculator_repl();
    } else {
        coroutines::run_calculator_repl();
    }

    return 0;
}
