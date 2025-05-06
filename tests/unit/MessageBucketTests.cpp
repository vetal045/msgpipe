#include "storage/MessageBucket.h"
#include <gtest/gtest.h>

using namespace msgpipe::storage;

TEST(MessageBucketTest, RejectsDuplicateAndOverflow) {
    MessageBucket bucket;

    // Insert 256 unique IDs
    for (uint64_t i = 1; i <= 256; ++i) {
        EXPECT_TRUE(bucket.insertIfAbsent(i));
    }

    EXPECT_FALSE(bucket.insertIfAbsent(1));
    EXPECT_FALSE(bucket.insertIfAbsent(999));
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
