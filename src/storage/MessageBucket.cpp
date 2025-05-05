#include "storage/MessageBucket.h"

namespace msgpipe::storage {

bool MessageBucket::insertIfAbsent(uint64_t messageId) {
    std::lock_guard<std::mutex> guard(lock_);

    for (auto& entry : entries_) {
        uint64_t current = entry.id.load(std::memory_order_relaxed);
        if (current == messageId) {
            return false;
        }
        if (current == 0) {
            entry.id.store(messageId, std::memory_order_release);
            return true;
        }
    }

    return false;
}

bool MessageBucket::exists(uint64_t messageId) const {
    std::lock_guard<std::mutex> guard(lock_);

    for (const auto& entry : entries_) {
        if (entry.id.load(std::memory_order_acquire) == messageId) {
            return true;
        }
    }

    return false;
}

} // namespace msgpipe::storage
