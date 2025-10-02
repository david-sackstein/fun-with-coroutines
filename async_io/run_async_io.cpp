#include "Fd.h"
#include "Selector.h"

#include <functional>
#include <unistd.h>
#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

void run_async_io(){
    Fd std_in(STDIN_FILENO);

    std::vector<std::reference_wrapper<Fd>> my_fds {std::ref(std_in)};

    Selector selector(my_fds);

    // Demonstrate external stop: background thread cancels after delay
    std::thread stopper([&] {
        std::this_thread::sleep_for(10s);
        selector.stop();
        std::cout << "[stopper] stop requested" << std::endl;
    });

    while (selector.is_running()) {
        auto ready = selector.wait_for_fds();
        for (auto &fdRef : ready) {
            if (fdRef.get().get() == std_in.get()) {
                std::string line;
                std::getline(std::cin, line);
                std::cout << "Read line: " << line << std::endl;
            }
        }
    }

    stopper.join();
    std::cout << "Stopped" << std::endl;
}