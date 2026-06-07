#include "common/event_loop/EventLoop.h"
#include "coroutines/3. async_tasks/MarshalledTask.h"
#include "coroutines/3. async_tasks/UnmarshalledTask.h"
#include "no-coroutines/3. async_tasks/MarshalledTask.h"
#include "no-coroutines/3. async_tasks/UnmarshalledTask.h"
#include "tests/Side.h"

#include <gtest/gtest.h>

#include <set>
#include <string>
#include <vector>

namespace {

std::vector<std::string> collect_unmarshalled(const Side side) {
    if (side == Side::Coroutines) {
        return coroutines::collect_unmarshalled_thread_ids();
    }
    return no_coroutines::collect_unmarshalled_thread_ids();
}

std::vector<std::string> collect_marshalled(const Side side) {
    EventLoop loop;
    if (side == Side::Coroutines) {
        return coroutines::collect_marshalled_thread_ids(loop);
    }
    return no_coroutines::collect_marshalled_thread_ids(loop);
}

class Demo3AsyncTasks : public testing::TestWithParam<Side> {};

TEST_P(Demo3AsyncTasks, UnmarshalledSample) {
    const std::vector<std::string> thread_ids = collect_unmarshalled(GetParam());
    ASSERT_EQ(thread_ids.size(), 3U);

    const std::set<std::string> unique_ids(thread_ids.begin(), thread_ids.end());
    EXPECT_GT(unique_ids.size(), 1U);
}

TEST_P(Demo3AsyncTasks, MarshalledSample) {
    const std::vector<std::string> thread_ids = collect_marshalled(GetParam());
    ASSERT_EQ(thread_ids.size(), 4U);

    for (size_t index = 1; index < thread_ids.size(); ++index) {
        EXPECT_EQ(thread_ids[index], thread_ids.front());
    }
}

INSTANTIATE_TEST_SUITE_P(Side, Demo3AsyncTasks, testing::Values(Side::Coroutines, Side::NoCoroutines),
                         [](const testing::TestParamInfo<Side> &info) { return side_name(info.param); });

}
