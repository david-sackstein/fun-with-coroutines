#include "common/io/print.h"
#include "common/testing/Delays.h"
#include "coroutines/03-async-tasks/AsyncTask.h"
#include "coroutines/03-async-tasks/UnmarshalledTask.h"
#include "coroutines/return_types/NeverSuspendCoroutine.h"

#include <thread>
#include <vector>

namespace coroutines {

namespace {

NeverSuspendCoroutine unmarshalled_task(std::vector<std::string> &thread_ids) {
    thread_ids.push_back(io::format_thread_id());

    co_await AsyncTask{};
    thread_ids.push_back(io::format_thread_id());

    co_await AsyncTask{};
    thread_ids.push_back(io::format_thread_id());
}

}

std::vector<std::string> collect_unmarshalled_thread_ids() {
    std::vector<std::string> thread_ids;
    [[maybe_unused]] auto task = unmarshalled_task(thread_ids);
    std::this_thread::sleep_for(testing_delay::unmarshalled_wait);
    return thread_ids;
}

}
