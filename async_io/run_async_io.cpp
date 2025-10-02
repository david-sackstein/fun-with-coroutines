#include "Selector.h"

#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

void run_async_io(){
    Selector selector({
        {STDIN_FILENO, [](int fd) {
            std::string line;
            std::getline(std::cin, line);
            std::cout << "Read line: " << line << std::endl;
        }}
    });

    // Demonstrate external stop: background thread cancels after delay
    std::thread stopper([&] {
        std::this_thread::sleep_for(10s);
        selector.stop();
        std::cout << "[stopper] stop requested" << std::endl;
    });

    selector.run();

    stopper.join();
    std::cout << "Stopped" << std::endl;
}