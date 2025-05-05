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

    bool insertIfAbsent(const msgpipe::protocol::Message& msg);
    bool exists(uint64_t messageId) const;

private:
    MessageBucket** buckets_;
    std::size_t bucketCount_;
};

} // namespace msgpipe::storage
