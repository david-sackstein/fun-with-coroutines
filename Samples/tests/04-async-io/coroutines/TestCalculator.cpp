#include "tests/04-async-io/coroutines/TestCalculator.h"

#include "common/pipe/Pipe.h"
#include "common/reactor/Reactor.h"
#include "common/testing/Delays.h"
#include "coroutines/04-async-io/calc/CalcClient.h"
#include "coroutines/04-async-io/calc/CalcServer.h"
#include "common/testing/CoutCapture.h"

#include <string>
#include <thread>

#include <unistd.h>

TEST_F(Demo04CalculatorCoroutines, Addition) {
    const std::string output = run_calculator_session("23+5\n");
    EXPECT_NE(output.find("28"), std::string::npos);
    EXPECT_NE(output.find("Response verified successfully"), std::string::npos);
}

TEST_F(Demo04CalculatorCoroutines, Subtraction) {
    const std::string output = run_calculator_session("10-3\n");
    EXPECT_NE(output.find("7"), std::string::npos);
    EXPECT_NE(output.find("Response verified successfully"), std::string::npos);
}

TEST_F(Demo04CalculatorCoroutines, Multiplication) {
    const std::string output = run_calculator_session("4*5\n");
    EXPECT_NE(output.find("20"), std::string::npos);
    EXPECT_NE(output.find("Response verified successfully"), std::string::npos);
}

TEST_F(Demo04CalculatorCoroutines, Division) {
    const std::string output = run_calculator_session("20/4\n");
    EXPECT_NE(output.find("5"), std::string::npos);
    EXPECT_NE(output.find("Response verified successfully"), std::string::npos);
}

std::string Demo04CalculatorCoroutines::run_calculator_session(const std::string_view input_line) {
    Pipe input_pipe;
    Pipe pipe_client_to_server;
    Pipe pipe_server_to_client;

    if (const ssize_t written = write(input_pipe.write_fd(), input_line.data(), input_line.size()); written < 0) {
        return {};
    }
    close(input_pipe.write_fd());

    Reactor reactor;

    const coroutines::CalcServer server(reactor, pipe_client_to_server.read_fd(), pipe_server_to_client.write_fd());

    const coroutines::CalcClient client(
        reactor, input_pipe.read_fd(),
        pipe_client_to_server.write_fd(),
        pipe_server_to_client.read_fd());

    [[maybe_unused]] const auto server_task = server.run();
    [[maybe_unused]] const auto client_task = client.run();

    std::thread stopper([&reactor] {
        std::this_thread::sleep_for(testing_delay::reactor_safety);
        reactor.stop();
    });
    stopper.detach();

    CoutCapture capture;
    reactor.run();
    return capture.str();
}
