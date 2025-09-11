#include "tests/03-async-tasks/coroutines/TestAsyncTasks.h"

#include <set>

TEST_F(Demo03AsyncTasksCoroutines, UnmarshalledSample) {
    const std::vector<std::string> thread_ids = collect_unmarshalled_thread_ids();
    ASSERT_EQ(thread_ids.size(), 3U);
    EXPECT_GT(std::set(thread_ids.begin(), thread_ids.end()).size(), 1U);
}

TEST_F(Demo03AsyncTasksCoroutines, MarshalledSample) {
    EventLoop loop;
    const std::vector<std::string> thread_ids = collect_marshalled_thread_ids(loop);
    ASSERT_EQ(thread_ids.size(), 4U);
    for (size_t index = 1; index < thread_ids.size(); ++index) {
        EXPECT_EQ(thread_ids[index], thread_ids.front());
    }
}

std::vector<std::string> Demo03AsyncTasksCoroutines::collect_unmarshalled_thread_ids() {
    return coroutines::collect_unmarshalled_thread_ids();
}

std::vector<std::string> Demo03AsyncTasksCoroutines::collect_marshalled_thread_ids(EventLoop &loop) {
    return coroutines::collect_marshalled_thread_ids(loop);
}
