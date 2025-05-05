#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>

namespace msgpipe::storage {

/**
 * @class MessageBucket
 * @brief Thread-safe fixed-size bucket for storing unique Message IDs.
 */
class MessageBucket {
public:
    static constexpr std::size_t kMaxEntries = 4;

    /**
     * @brief Attempts to insert ID if it does not exist yet.
     * @param messageId ID to insert
     * @return true if inserted, false if duplicate or full
     */
    bool insertIfAbsent(uint64_t messageId);

    /**
     * @brief Checks whether ID exists in the bucket.
     * @param messageId ID to check
     * @return true if found
     */
    bool exists(uint64_t messageId) const;

private:
    struct Entry {
        std::atomic<uint64_t> id{0};
    };

    mutable std::mutex lock_;
    Entry entries_[kMaxEntries];
};

} // namespace msgpipe::storage
