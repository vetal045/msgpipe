/**
 * @file MessageBucketStore.h
 * @brief Contains the MessageBucketStore class for deduplication.
 */
#pragma once

#include "protocol/Message.h"
#include "storage/MessageBucket.h"

#include <cstddef>
#include <cstdint>

namespace msgpipe::storage {

/**
 * @class MessageBucketStore
 * @brief High-performance concurrent storage using array of MessageBucket objects.
 *
 * Each message is deduplicated based on its `id`. This class handles bucket allocation and dispatching.
 */
class MessageBucketStore {
public:
    explicit MessageBucketStore(std::size_t bucketCount);
    ~MessageBucketStore();

    /**
     * @brief Inserts message if its ID has not been seen before.
     * @param msg Parsed message
     * @return True if inserted (not duplicate), false otherwise
     */
    bool insertIfAbsent(const msgpipe::protocol::Message& msg);

    /**
     * @brief Checks whether a message ID has already been seen.
     * @param messageId ID to check
     * @return True if message exists, false otherwise
     */
    bool exists(uint64_t messageId) const;

private:
    MessageBucket** buckets_;
    std::size_t bucketCount_;
};

} // namespace msgpipe::storage