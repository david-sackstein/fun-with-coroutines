#include "reactor/Reactor.h"
#include "echo/Pipe.h"
#include "echo/EchoClient.h"
#include "echo/EchoServer.h"
#include <print>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

void run_echo_demo() {
    std::print("=== Echo Server Demo ===\n");
    std::print("Type messages (ending with newline), they will be echoed through pipes\n");
    std::print("Press Ctrl+D to exit (or wait 30s for timeout)\n\n");
    
    // Set stdin to non-blocking
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    
    // Create two pipes (both will be non-blocking)
    Pipe pipe1;  // Client writes, Server reads
    Pipe pipe2;  // Server writes, Client reads
    
    // Create _reactor
    Reactor reactor;
    
    // Create and start client and server coroutines
    EchoClient client(reactor, STDIN_FILENO, pipe1.write_fd(), pipe2.read_fd());
    EchoServer server(reactor, pipe1.read_fd(), pipe2.write_fd());
    
    auto client_task = client.run();
    auto server_task = server.run();
    
    // Demonstrate external stop: background thread cancels after delay
    std::thread stopper([&] {
        std::this_thread::sleep_for(10s);
        std::print("\n[Stopper] Timeout - stopping _reactor\n");
        reactor.stop();
    });
    
    // Run the _reactor
    reactor.run();
    
    stopper.join();
    
    std::print("\n=== Echo Server Demo Finished ===\n");
}
