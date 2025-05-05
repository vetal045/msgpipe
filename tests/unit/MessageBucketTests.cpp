#include "storage/MessageBucket.h"
#include <gtest/gtest.h>

using namespace msgpipe::storage;

TEST(MessageBucketTest, InsertUniqueIds) {
    MessageBucket bucket;
    EXPECT_TRUE(bucket.insertIfAbsent(1));
    EXPECT_TRUE(bucket.insertIfAbsent(2));
    EXPECT_TRUE(bucket.insertIfAbsent(3));
    EXPECT_TRUE(bucket.insertIfAbsent(4));
    EXPECT_FALSE(bucket.insertIfAbsent(5)); // full
}

TEST(MessageBucketTest, InsertDuplicateId) {
    MessageBucket bucket;
    EXPECT_TRUE(bucket.insertIfAbsent(42));
    EXPECT_FALSE(bucket.insertIfAbsent(42));
}

TEST(MessageBucketTest, ExistsCheck) {
    MessageBucket bucket;
    bucket.insertIfAbsent(100);
    EXPECT_TRUE(bucket.exists(100));
    EXPECT_FALSE(bucket.exists(999));
}
