// tests/unit/test_message_queue.cpp
#include "storage/MessageQueue.h"
#include "protocol/Message.h"
#include <gtest/gtest.h>

using namespace msgpipe::storage;
using namespace msgpipe::protocol;

TEST(MessageQueueTest, SinglePushPop) {
    MessageQueue queue(8);
    Message msg{sizeof(Message), 1, 42, 10};

    EXPECT_TRUE(queue.tryPush(msg));

    Message out{};
    EXPECT_TRUE(queue.tryPop(out));
    EXPECT_EQ(out.id, msg.id);
    EXPECT_EQ(out.data, msg.data);
}

TEST(MessageQueueTest, FillQueueThenFail) {
    MessageQueue queue(4);
    Message msg{sizeof(Message), 1, 100, 10};

    for (int i = 0; i < 4; ++i) {
        msg.id = 100 + i;
        EXPECT_TRUE(queue.tryPush(msg)) << "Failed on iteration " << i;
    }

    msg.id = 999;
    EXPECT_FALSE(queue.tryPush(msg));
}

TEST(MessageQueueTest, OrderPreserved) {
    MessageQueue queue(8);

    for (int i = 0; i < 5; ++i) {
        Message msg{sizeof(Message), 1, static_cast<uint64_t>(100 + i), 10};
        EXPECT_TRUE(queue.tryPush(msg));
    }

    for (int i = 0; i < 5; ++i) {
        Message out{};
        EXPECT_TRUE(queue.tryPop(out));
        EXPECT_EQ(out.id, 100 + i);
    }
}

TEST(MessageQueueTest, PopFromEmptyFails) {
    MessageQueue queue(4);
    Message out{};
    EXPECT_FALSE(queue.tryPop(out));
}
