#pragma once

#include "protocol/Message.h"
#include <atomic>
#include <cstddef>

namespace msgpipe::storage {

/**
 * @class MessageQueue
 * @brief Lock-free fixed-size multiple-producer, single-consumer message queue.
 *
 * Designed for concurrent push from multiple UDP threads and pop from a single TCP thread.
 */
class MessageQueue {
public:
    /**
     * @brief Constructs the queue with given capacity.
     * @param capacity Number of elements that can be stored
     */
    explicit MessageQueue(std::size_t capacity);

    ~MessageQueue();

    /**
     * @brief Attempts to enqueue a message.
     * @param msg Message to enqueue
     * @return true if successful, false if queue is full
     */
    bool tryPush(const msgpipe::protocol::Message& msg);

    /**
     * @brief Attempts to dequeue a message.
     * @param outMsg Reference to fill with the popped message
     * @return true if successful, false if queue is empty
     */
    bool tryPop(msgpipe::protocol::Message& outMsg);

private:
    struct Slot {
        std::atomic<bool> ready;
        msgpipe::protocol::Message value;
    };

    Slot* buffer_;
    const std::size_t capacity_;

    std::atomic<std::size_t> head_; ///< written by consumer
    std::atomic<std::size_t> tail_; ///< written by producers
};

} // namespace msgpipe::storage
