#include "reactor/Reactor.h"
#include "echo/Pipe.h"
#include "echo/EchoClient.h"
#include "echo/EchoServer.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

void run_echo_demo() {
    std::cout << "=== Echo Server Demo ===" << std::endl;
    std::cout << "Type messages, they will be echoed through pipes" << std::endl;
    std::cout << "Press Ctrl+D to exit (or wait 30s for timeout)" << std::endl << std::endl;
    
    // Create two pipes
    Pipe pipe1;  // Client writes, Server reads
    Pipe pipe2;  // Server writes, Client reads
    
    // Create reactor
    Reactor reactor;
    
    // Create and start client and server coroutines
    EchoClient client(reactor, STDIN_FILENO, pipe1.write_fd(), pipe2.read_fd());
    EchoServer server(reactor, pipe1.read_fd(), pipe2.write_fd());
    
    auto client_task = client.run();
    auto server_task = server.run();
    
    // Demonstrate external stop: background thread cancels after delay
    std::thread stopper([&] {
        std::this_thread::sleep_for(30s);
        std::cout << "\n[Stopper] Timeout - stopping reactor" << std::endl;
        reactor.stop();
    });
    
    // Run the reactor
    reactor.run();
    
    stopper.join();
    
    std::cout << "\n=== Echo Server Demo Finished ===" << std::endl;
}

