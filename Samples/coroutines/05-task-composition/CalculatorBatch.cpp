#include "common/pipe/Pipe.h"
#include "common/reactor/Reactor.h"
#include "common/reactor/ReactorStopper.h"
#include "common/testing/Delays.h"
#include "coroutines/04-async-io/calc/CalcServer.h"
#include "coroutines/05-task-composition/CalculatorBatch.h"
#include "coroutines/05-task-composition/SolveQuiz.h"

#include <array>
#include <string_view>
#include <vector>

namespace coroutines {

// Wire the server and solve_quiz together, run the Reactor, and return the collected answers.
std::vector<int> run_calculator_batch() {
    constexpr std::array<std::string_view, 4> quiz_questions = {"2+2\n", "10-3\n", "4*5\n", "20/4\n"};

    const Pipe pipe_client_to_server;
    const Pipe pipe_server_to_client;

    Reactor reactor;

    const CalcServer server(reactor, pipe_client_to_server.read_fd(), pipe_server_to_client.write_fd());
    const auto server_coro = server.run();

    auto task = solve_quiz(
        reactor,
        pipe_client_to_server.write_fd(), pipe_server_to_client.read_fd(),
        quiz_questions);

    task.start();

    const ReactorStopper stopper(reactor, testing_delay::reactor_safety);

    reactor.run();

    return task.result();
}

}
