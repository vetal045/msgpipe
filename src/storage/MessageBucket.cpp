#include "storage/MessageBucket.h"

namespace msgpipe::storage {

bool MessageBucket::insertIfAbsent(uint64_t id) {
    std::lock_guard<std::mutex> lock(mutex_);

    for (std::size_t i = 0; i < count_; ++i) {
        if (ids_[i] == id) {
            return false;
        }
    }

    if (count_ < kCapacity) {
        ids_[count_++] = id;
        return true;
    }

    return false; // full
}

bool MessageBucket::exists(uint64_t id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    for (std::size_t i = 0; i < count_; ++i) {
        if (ids_[i] == id) {
            return true;
        }
    }

    return false;
}

} // namespace msgpipe::storage
