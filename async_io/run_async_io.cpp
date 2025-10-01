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
    std::unique_ptr<Selector::Work> work = std::make_unique<Selector::Work>(selector);
    std::thread stopper([&] {
        std::this_thread::sleep_for(2s);
        work.reset();
        std::cout << "[stopper] stop requested" << std::endl;
    });

    while (selector.get_outstanding_work() > 0) {
        auto ready = selector.wait_for_fds();
        for (auto &fdRef : ready) {
            if (fdRef.get().get() == std_in.get()) {
                std::string line;
                std::getline(std::cin, line);
                std::cout << "Read line: " << line << std::endl;
                work.reset();
                break;
            }
        }
    }

    stopper.join();
    std::cout << "Stopped" << std::endl;
}