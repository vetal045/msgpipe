#include "storage/MessageBucketStore.h"
#include "protocol/Message.h"

#include <gtest/gtest.h>

using namespace msgpipe::storage;
using namespace msgpipe::protocol;

TEST(MessageBucketStoreTest, InsertAndExists) {
    MessageBucketStore store(64);
    Message msg{sizeof(Message), 1, 42, 10};

    EXPECT_TRUE(store.insertIfAbsent(msg));
    EXPECT_FALSE(store.insertIfAbsent(msg)); // Duplicate
    EXPECT_TRUE(store.exists(42));
    EXPECT_FALSE(store.exists(999));
}

TEST(MessageBucketStoreTest, ManyIds) {
    MessageBucketStore store(1024);

    for (int i = 0; i < 1000; ++i) {
        Message msg{sizeof(Message), 1, static_cast<uint64_t>(1000 + i), 10};
        EXPECT_TRUE(store.insertIfAbsent(msg)) << "Failed on ID " << (1000 + i);
    }

    for (int i = 0; i < 1000; ++i) {
        EXPECT_TRUE(store.exists(1000 + i)) << "Missing ID " << (1000 + i);
    }
}
