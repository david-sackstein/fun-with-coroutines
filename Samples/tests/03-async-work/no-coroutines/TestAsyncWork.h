#pragma once

#include "common/event_loop/EventLoop.h"
#include "no-coroutines/03-async-work/MarshalledWork.h"
#include "no-coroutines/03-async-work/UnmarshalledWork.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

class Demo03AsyncWorkNoCoroutines : public testing::Test {
protected:
    [[nodiscard]] static std::vector<std::string> collect_unmarshalled_thread_ids();
    [[nodiscard]] static std::vector<std::string> collect_marshalled_thread_ids(EventLoop &loop);
};
