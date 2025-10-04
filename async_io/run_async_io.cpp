#include "Selector.h"
#include "AsyncRead.h"
#include "CoroutineObject.h"

#include <unistd.h>
#include <iostream>
#include <string>

CoroutineObject read_lines(Selector& selector) {
    std::cout << "Starting coroutine-based I/O..." << std::endl;

    for (int i = 0; i < 3; ++i) {
        std::cout << "\nWaiting for input (iteration " << (i + 1) << ")..." << std::endl;
        
        // Wait for stdin to be readable (control returns to select here)
        co_await AsyncRead{selector, STDIN_FILENO};
        
        // Now it's ready, do the read
        std::string line;
        std::getline(std::cin, line);
        std::cout << "Read line: " << line << std::endl;
    }
    
    std::cout << "\nDone reading, stopping selector" << std::endl;
    selector.stop();
}

void run_async_io(){
    Selector selector;

    // Start the coroutine - it will suspend at each co_await
    auto task = read_lines(selector);
    
    // Run the event loop - it will dispatch I/O events
    selector.run();
    
    std::cout << "Selector stopped cleanly" << std::endl;
}