#pragma once

#include <mutex>
#include <cstdint>

namespace msgpipe::storage {

/**
 * @class MessageBucket
 * @brief Stores up to 256 unique message IDs for deduplication.
 *
 * Thread-safe via internal coarse-grained locking.
 * Designed for high-performance insertion and lookup in fixed-size slot.
 */
class MessageBucket {
public:
    /**
     * @brief Insert a new ID if it doesn't already exist.
     * @param id Message ID
     * @return true if inserted, false if duplicate or full
     */
    bool insertIfAbsent(uint64_t id);

    /**
     * @brief Check whether an ID already exists.
     * @param id Message ID
     * @return true if present, false otherwise
     */
    bool exists(uint64_t id) const;

private:
    static constexpr std::size_t kCapacity = 256;

    mutable std::mutex mutex_;             ///< Guards access to ids_ and count_
    uint64_t ids_[kCapacity]{};            ///< Stored message IDs
    std::size_t count_{0};                 ///< Number of IDs currently stored
};

} // namespace msgpipe::storage
