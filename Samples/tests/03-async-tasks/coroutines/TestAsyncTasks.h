#pragma once

#include "common/event_loop/EventLoop.h"
#include "coroutines/03-async-tasks/MarshalledTask.h"
#include "coroutines/03-async-tasks/UnmarshalledTask.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

class Demo03AsyncTasksCoroutines : public testing::Test {
protected:
    [[nodiscard]] static std::vector<std::string> collect_unmarshalled_thread_ids();
    [[nodiscard]] static std::vector<std::string> collect_marshalled_thread_ids(EventLoop &loop);
};
