#include <gtest/gtest.h>
#include "../../client/src/SafeQueue.hpp"
#include <string>

TEST(ThreadSafeQueueAdvanced, LargeVolume) {
    ThreadSafeQueue<int> queue;
    const int count = 10000;
    
    for (int i = 0; i < count; ++i) {
        queue.push(i);
    }
    
    EXPECT_EQ(queue.size(), count);
}

TEST(ThreadSafeQueueAdvanced, FIFO) {
    ThreadSafeQueue<int> queue;
    
    queue.push(1);
    queue.push(2);
    queue.push(3);
    queue.push(4);
    queue.push(5);
    
    int value;
    EXPECT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value, 1);
    
    EXPECT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value, 2);
    
    EXPECT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value, 3);
}

TEST(ThreadSafeQueueAdvanced, ComplexTypes) {
    struct Message {
        int id;
        std::string text;
        float value;
    };
    
    ThreadSafeQueue<Message> queue;
    
    Message msg1{1, "Hello", 3.14f};
    Message msg2{2, "World", 2.71f};
    
    queue.push(msg1);
    queue.push(msg2);
    
    Message result;
    EXPECT_TRUE(queue.try_pop(result));
    EXPECT_EQ(result.id, 1);
    EXPECT_EQ(result.text, "Hello");
    EXPECT_FLOAT_EQ(result.value, 3.14f);
}

TEST(ThreadSafeQueueAdvanced, RapidPushPop) {
    ThreadSafeQueue<int> queue;
    
    for (int i = 0; i < 100; ++i) {
        queue.push(i);
        
        if (i % 2 == 0) {
            int value;
            queue.try_pop(value);
        }
    }
    
    EXPECT_GT(queue.size(), 0u);
}

TEST(ThreadSafeQueueAdvanced, SizeTracking) {
    ThreadSafeQueue<int> queue;
    
    EXPECT_EQ(queue.size(), 0u);
    
    queue.push(1);
    EXPECT_EQ(queue.size(), 1u);
    
    queue.push(2);
    EXPECT_EQ(queue.size(), 2u);
    
    int value;
    queue.try_pop(value);
    EXPECT_EQ(queue.size(), 1u);
    
    queue.try_pop(value);
    EXPECT_EQ(queue.size(), 0u);
}

TEST(ThreadSafeQueueAdvanced, EmptyCheck) {
    ThreadSafeQueue<int> queue;
    
    EXPECT_TRUE(queue.empty());
    
    queue.push(1);
    EXPECT_FALSE(queue.empty());
    
    int value;
    queue.try_pop(value);
    EXPECT_TRUE(queue.empty());
}

TEST(ThreadSafeQueueAdvanced, ConstRefPush) {
    ThreadSafeQueue<std::string> queue;
    
    const std::string str = "Const String";
    queue.push(str);
    
    std::string result;
    EXPECT_TRUE(queue.try_pop(result));
    EXPECT_EQ(result, str);
}

TEST(ThreadSafeQueueAdvanced, RvaluePush) {
    ThreadSafeQueue<std::string> queue;
    
    queue.push(std::string("Rvalue"));
    
    std::string result;
    EXPECT_TRUE(queue.try_pop(result));
    EXPECT_EQ(result, "Rvalue");
}

TEST(ThreadSafeQueueAdvanced, AlternatingOperations) {
    ThreadSafeQueue<int> queue;
    
    queue.push(1);
    queue.push(2);
    
    int v1;
    EXPECT_TRUE(queue.try_pop(v1));
    EXPECT_EQ(v1, 1);
    
    queue.push(3);
    queue.push(4);
    
    int v2;
    EXPECT_TRUE(queue.try_pop(v2));
    EXPECT_EQ(v2, 2);
    
    EXPECT_EQ(queue.size(), 2u);
}

TEST(ThreadSafeQueueAdvanced, PopUntilEmpty) {
    ThreadSafeQueue<int> queue;
    
    for (int i = 0; i < 5; ++i) {
        queue.push(i);
    }
    
    int value;
    int count = 0;
    while (queue.try_pop(value)) {
        count++;
    }
    
    EXPECT_EQ(count, 5);
    EXPECT_TRUE(queue.empty());
}
