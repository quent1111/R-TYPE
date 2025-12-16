#include <gtest/gtest.h>
#include "../../client/include/common/SafeQueue.hpp"
#include <thread>
#include <vector>

TEST(ThreadSafeQueue, PushAndPop) {
    ThreadSafeQueue<int> queue;

    queue.push(1);
    queue.push(2);
    queue.push(3);

    EXPECT_EQ(queue.size(), 3u);

    int value;
    EXPECT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value, 1);

    EXPECT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value, 2);

    EXPECT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value, 3);

    EXPECT_TRUE(queue.empty());
}

TEST(ThreadSafeQueue, EmptyQueue) {
    ThreadSafeQueue<int> queue;

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0u);

    int value;
    EXPECT_FALSE(queue.try_pop(value));
}

TEST(ThreadSafeQueue, MoveSemantics) {
    ThreadSafeQueue<std::string> queue;

    std::string str = "Hello";
    queue.push(std::move(str));

    std::string result;
    EXPECT_TRUE(queue.try_pop(result));
    EXPECT_EQ(result, "Hello");
}

TEST(ThreadSafeQueue, ThreadSafety) {
    ThreadSafeQueue<int> queue;
    const int num_threads = 4;
    const int items_per_thread = 100;

    std::vector<std::thread> producers;
    for (int t = 0; t < num_threads; ++t) {
        producers.emplace_back([&queue, t, items_per_thread]() {
            for (int i = 0; i < items_per_thread; ++i) {
                queue.push(t * items_per_thread + i);
            }
        });
    }

    for (auto& thread : producers) {
        thread.join();
    }

    EXPECT_EQ(queue.size(), num_threads * items_per_thread);

    std::vector<int> consumed_values;
    std::mutex consumed_mutex;

    std::vector<std::thread> consumers;
    for (int t = 0; t < num_threads; ++t) {
        consumers.emplace_back([&queue, &consumed_values, &consumed_mutex]() {
            int value;
            while (queue.try_pop(value)) {
                std::lock_guard<std::mutex> lock(consumed_mutex);
                consumed_values.push_back(value);
            }
        });
    }

    for (auto& thread : consumers) {
        thread.join();
    }

    EXPECT_EQ(consumed_values.size(), num_threads * items_per_thread);
    EXPECT_TRUE(queue.empty());
}
