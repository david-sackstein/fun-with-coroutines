#include "Selector.h"
#include "AsyncRead.h"
#include "AsyncIoCoroutine.h"

#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std::chrono_literals;

AsyncIoCoroutine read_lines(Selector& selector) {
    std::cout << "Starting coroutine-based I/O..." << std::endl;

    while(true) {
        // Wait for stdin to be readable (control returns to select here)
        co_await AsyncRead{selector, STDIN_FILENO};
        
        // Now it's ready, do the read
        std::string line;
        std::getline(std::cin, line);
        std::cout << "Read line: " << line << std::endl;
        break;
    }
}

void run_async_io(){
    Selector selector;

    // Start the coroutine - it will suspend at each co_await
    auto task = read_lines(selector);
    
    // Demonstrate external stop: background thread cancels after delay
    std::thread stopper([&] {
        std::this_thread::sleep_for(2s);
        selector.stop();
        std::cout << "[stopper] stop requested" << std::endl;
    });
    
    // Run the event loop - it will dispatch I/O events
    selector.run();
    
    stopper.join();
    
    std::cout << "Selector stopped cleanly" << std::endl;
}