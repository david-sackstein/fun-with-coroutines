#include <functional>
#include <unistd.h>
#include "Fd.h"
#include "wait_for_fds.h"

using namespace std::chrono_literals;

void run_async_io(){
    Fd std_in(STDIN_FILENO);

    std::vector<std::reference_wrapper<Fd>> my_fds {std_in};
    wait_for_fds(my_fds, 1000s);

    std::cout << "Success" << std::endl;
}