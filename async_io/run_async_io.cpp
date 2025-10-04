#include "reactor/Reactor.h"
#include "async/AsyncReadBuffer.h"
#include "async/AsyncIoCoroutine.h"

#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std::chrono_literals;

AsyncIoCoroutine read_data(Reactor& reactor) {
    std::cout << "Starting coroutine-based I/O..." << std::endl;

    char buffer[1024];
    size_t total_bytes = 0;
    
    while(true) {
        // Wait for stdin to be readable and read into buffer
        ssize_t n = co_await AsyncReadBuffer{reactor, STDIN_FILENO, std::span<char>(buffer)};
        
        if (n <= 0) {
            std::cout << "EOF or error (n=" << n << ")" << std::endl;
            break;
        }
        
        total_bytes += n;
        std::cout << "Read " << n << " bytes: " 
                  << std::string_view(buffer, n) << std::endl;
    }
    
    std::cout << "Total bytes read: " << total_bytes << std::endl;
}

void run_async_io(){
    Reactor reactor;

    // Start the coroutine - it will suspend at each co_await
    auto task = read_data(reactor);
    
    // Demonstrate external stop: background thread cancels after delay
    std::thread stopper([&] {
        std::this_thread::sleep_for(10s);
        reactor.stop();
        std::cout << "[stopper] stop requested" << std::endl;
    });
    
    // Run the event loop - it will dispatch I/O events
    reactor.run();
    
    stopper.join();
    
    std::cout << "Reactor stopped cleanly" << std::endl;
}