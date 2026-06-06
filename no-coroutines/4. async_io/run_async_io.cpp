#include "common/io/print.h"
#include "common/pipe/Pipe.h"
#include "common/reactor/Reactor.h"
#include "no-coroutines/4. async_io/calc/CalcClient.h"
#include "no-coroutines/4. async_io/calc/CalcServer.h"

#include <thread>

#include <fcntl.h>
#include <unistd.h>

namespace no_coroutines {

using namespace std::chrono_literals;

void setup_stdin();
std::thread start_stopper_thread(Reactor &reactor);

void run_async_io() {
    io::print("=== Calculator Demo ===\n");
    io::print("Enter expressions like 23+5 (one +, -, *, / operation, ending with newline)\n");
    io::print("Press Ctrl+D to exit (or wait 10s for timeout)\n\n");

    setup_stdin();
    
    // Create two pipes (both will be non-blocking)
    const Pipe pipe_client_to_server;  // Client writes, Server reads
    const Pipe pipe_server_to_client;  // Server writes, Client reads
    
    // Create reactor
    Reactor reactor;
    
    // Create and start client and server
    CalcClient client(reactor, STDIN_FILENO, pipe_client_to_server.write_fd(), pipe_server_to_client.read_fd());
    CalcServer server(reactor, pipe_client_to_server.read_fd(), pipe_server_to_client.write_fd());
    
    client.run();
    server.run();
    
    // Start timeout thread (detached - just a safety net)
    std::thread stopper = start_stopper_thread(reactor);
    stopper.detach();
    
    // Run the reactor and catch any exceptions
    try {
        reactor.run();
    } catch (const std::exception &e) {
        io::print("\n✗ Error: {}\n", e.what());
    }

    io::print("\n=== Calculator Demo Finished ===\n");
}

void setup_stdin() {
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

std::thread start_stopper_thread(Reactor &reactor) {
    return std::thread([&reactor] {
        std::this_thread::sleep_for(10s);
        io::print("\n[Stopper] Timeout - stopping reactor\n");
        reactor.stop();
    });
}

}
