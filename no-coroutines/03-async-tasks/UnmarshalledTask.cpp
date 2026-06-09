#include "common/io/print.h"
#include "common/testing/Delays.h"
#include "no-coroutines/03-async-tasks/AsyncTask.h"
#include "no-coroutines/03-async-tasks/UnmarshalledTask.h"

#include <memory>
#include <thread>
#include <vector>

namespace no_coroutines {

namespace {

void unmarshalled_task(std::shared_ptr<std::vector<std::string>> thread_ids) {
    auto runner = std::make_shared<AsyncTask>();

    thread_ids->push_back(io::format_thread_id());

    runner->run_async_operation([runner, thread_ids] {
        thread_ids->push_back(io::format_thread_id());

        runner->run_async_operation([thread_ids] {
            thread_ids->push_back(io::format_thread_id());
        });
    });
}

}

std::vector<std::string> collect_unmarshalled_thread_ids() {
    auto thread_ids = std::make_shared<std::vector<std::string>>();
    unmarshalled_task(thread_ids);
    std::this_thread::sleep_for(testing_delay::unmarshalled_wait);
    return *thread_ids;
}

}
