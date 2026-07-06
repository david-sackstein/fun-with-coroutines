#include "tests/03-async-work/no-coroutines/TestAsyncWork.h"

#include <set>

TEST_F(Demo03AsyncWorkNoCoroutines, UnmarshalledSample) {
    const std::vector<std::string> thread_ids = collect_unmarshalled_thread_ids();
    ASSERT_EQ(thread_ids.size(), 3U);
    EXPECT_GT(std::set(thread_ids.begin(), thread_ids.end()).size(), 1U);
}

TEST_F(Demo03AsyncWorkNoCoroutines, MarshalledSample) {
    EventLoop loop;
    const std::vector<std::string> thread_ids = collect_marshalled_thread_ids(loop);
    ASSERT_EQ(thread_ids.size(), 4U);
    for (size_t index = 1; index < thread_ids.size(); ++index) {
        EXPECT_EQ(thread_ids[index], thread_ids.front());
    }
}

std::vector<std::string> Demo03AsyncWorkNoCoroutines::collect_unmarshalled_thread_ids() {
    return no_coroutines::collect_unmarshalled_thread_ids();
}

std::vector<std::string> Demo03AsyncWorkNoCoroutines::collect_marshalled_thread_ids(EventLoop &loop) {
    return no_coroutines::collect_marshalled_thread_ids(loop);
}
