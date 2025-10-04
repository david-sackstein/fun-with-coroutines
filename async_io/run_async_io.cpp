#include "reactor/Reactor.h"
#include "async/AsyncBuffer.h"
#include "async/AsyncIoCoroutine.h"

#include <unistd.h>
#include <print>
#include <string>
#include <thread>

using namespace std::chrono_literals;

AsyncIoCoroutine read_data(Reactor& reactor) {
    std::print("Starting coroutine-based I/O...\n");

    char buffer[1024];
    size_t total_bytes = 0;
    
    while(true) {
        // Wait for stdin to be readable and read into buffer
        ssize_t n = co_await AsyncReadBuffer<>{reactor, STDIN_FILENO, std::span<char>(buffer)};
        
        if (n <= 0) {
            std::print("EOF or error (n={})\n", n);
            break;
        }
        
        total_bytes += n;
        std::print("Read {} bytes: {}\n", n, std::string_view(buffer, n));
    }
    
    std::print("Total bytes read: {}\n", total_bytes);
}

void run_async_io(){
    Reactor reactor;

    // Start the coroutine - it will suspend at each co_await
    auto task = read_data(reactor);
    
    // Demonstrate external stop: background thread cancels after delay
    std::thread stopper([&] {
        std::this_thread::sleep_for(10s);
        reactor.stop();
        std::print("[stopper] stop requested\n");
    });
    
    // Run the event loop - it will dispatch I/O events
    reactor.run();
    
    stopper.join();
    
    std::print("Reactor stopped cleanly\n");
}