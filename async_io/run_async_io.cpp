#include "Selector.h"

#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

void run_async_io(){
    std::vector<int> my_fds {STDIN_FILENO};

    Selector selector(my_fds);

    // Demonstrate external stop: background thread cancels after delay
    std::thread stopper([&] {
        std::this_thread::sleep_for(10s);
        selector.stop();
        std::cout << "[stopper] stop requested" << std::endl;
    });

    while (selector.is_running()) {
        auto ready = selector.wait_for_fds();
        for (int fd : ready) {
            if (fd == STDIN_FILENO) {
                std::string line;
                std::getline(std::cin, line);
                std::cout << "Read line: " << line << std::endl;
            }
        }
    }

    stopper.join();
    std::cout << "Stopped" << std::endl;
}