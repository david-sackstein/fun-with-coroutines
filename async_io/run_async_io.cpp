#include "Fd.h"
#include "Selector.h"

#include <functional>
#include <unistd.h>
#include <iostream>

using namespace std::chrono_literals;

void run_async_io(){
    Fd std_in(STDIN_FILENO);

    std::vector<std::reference_wrapper<Fd>> my_fds {std::ref(std_in)};

    Selector selector(my_fds, 1000s);

    auto ready = selector.wait_for_fds();
    (void)ready;

    std::cout << "Success" << std::endl;
}