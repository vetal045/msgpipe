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
 * Provides deduplication based on message ID. Manages ownership of MessageBucket instances.
 * Designed to be thread-safe and avoid dynamic container usage.
 */
class MessageBucketStore {
public:
    /**
     * @brief Constructs the store with a fixed number of buckets.
     * @param bucketCount Total number of buckets to allocate.
     */
    explicit MessageBucketStore(std::size_t bucketCount);

    /// @brief Frees all allocated buckets.
    ~MessageBucketStore();

    /**
     * @brief Attempts to insert a message based on its ID.
     * @param msg The message to insert.
     * @return true if the ID was unique and inserted; false if duplicate.
     */
    bool insertIfAbsent(const msgpipe::protocol::Message& msg);

    /**
     * @brief Checks whether the given message ID already exists.
     * @param messageId The message ID to check.
     * @return true if ID exists; false otherwise.
     */
    bool exists(uint64_t messageId) const;

private:
    /**
     * @struct BucketSlot
     * @brief RAII wrapper around single MessageBucket to avoid raw allocation.
     */
    struct BucketSlot {
        MessageBucket* ptr;

        BucketSlot() : ptr(new MessageBucket()) {}
        ~BucketSlot() { delete ptr; }

        BucketSlot(const BucketSlot&) = delete;
        BucketSlot& operator=(const BucketSlot&) = delete;

        BucketSlot(BucketSlot&& other) noexcept : ptr(other.ptr) { other.ptr = nullptr; }
        BucketSlot& operator=(BucketSlot&& other) noexcept {
            if (this != &other) {
                delete ptr;
                ptr = other.ptr;
                other.ptr = nullptr;
            }
            return *this;
        }

        MessageBucket* get() const { return ptr; }
    };

    BucketSlot* buckets_;          ///< Fixed-size array of bucket slots (heap-allocated).
    std::size_t bucketCount_;      ///< Total number of buckets.
};

} // namespace msgpipe::storage
