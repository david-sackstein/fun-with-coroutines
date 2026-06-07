#pragma once

#include "common/pipe/Pipe.h"
#include "common/reactor/Reactor.h"
#include "common/testing/Delays.h"
#include "coroutines/4. async_io/calc/CalcClient.h"
#include "coroutines/4. async_io/calc/CalcServer.h"

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <thread>

#include <unistd.h>

class Demo4CalculatorCoroutines : public testing::Test {
protected:
    [[nodiscard]] std::string run_calculator_session(const std::string_view input_line) const {
        Pipe input_pipe;
        Pipe pipe_client_to_server;
        Pipe pipe_server_to_client;

        if (const ssize_t written = write(input_pipe.write_fd(), input_line.data(), input_line.size()); written < 0) {
            return {};
        }
        close(input_pipe.write_fd());

        Reactor reactor;

        const coroutines::CalcServer server(reactor, pipe_client_to_server.read_fd(), pipe_server_to_client.write_fd());
        const coroutines::CalcClient client(reactor, input_pipe.read_fd(), pipe_client_to_server.write_fd(),
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

private:
    class CoutCapture {
    public:
        CoutCapture() : _old_buf(std::cout.rdbuf(_buffer.rdbuf())) {}

        ~CoutCapture() { std::cout.rdbuf(_old_buf); }

        [[nodiscard]] std::string str() const { return _buffer.str(); }

    private:
        std::stringstream _buffer;
        std::streambuf *_old_buf;
    };
};
