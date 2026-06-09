#include "tests/04-async-io/coroutines/Calculator.h"

TEST_F(Demo04CalculatorCoroutines, PipedExpression) {
    const std::string output = run_calculator_session("23+5\n");
    EXPECT_NE(output.find("28"), std::string::npos);
    EXPECT_NE(output.find("Response verified successfully"), std::string::npos);
}

