#pragma once

#include "common/event_loop/EventLoop.h"
#include "coroutines/3. async_tasks/MarshalledTask.h"
#include "coroutines/3. async_tasks/UnmarshalledTask.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

class Demo3AsyncTasksCoroutines : public testing::Test {
protected:
    [[nodiscard]] std::vector<std::string> collect_unmarshalled_thread_ids() const {
        return coroutines::collect_unmarshalled_thread_ids();
    }

    [[nodiscard]] std::vector<std::string> collect_marshalled_thread_ids(EventLoop &loop) const {
        return coroutines::collect_marshalled_thread_ids(loop);
    }
};
