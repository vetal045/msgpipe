#include "storage/MessageBucketStore.h"

namespace msgpipe::storage {

MessageBucketStore::MessageBucketStore(std::size_t bucketCount)
    : bucketCount_(bucketCount)
{
    buckets_ = static_cast<BucketSlot*>(::operator new[](bucketCount_ * sizeof(BucketSlot)));
    for (std::size_t i = 0; i < bucketCount_; ++i) {
        new (&buckets_[i]) BucketSlot(); // placement new
    }
}

MessageBucketStore::~MessageBucketStore() {
    for (std::size_t i = 0; i < bucketCount_; ++i) {
        buckets_[i].~BucketSlot();
    }
    ::operator delete[](buckets_);
}

bool MessageBucketStore::insertIfAbsent(const msgpipe::protocol::Message& msg) {
    std::size_t index = msg.id % bucketCount_;
    return buckets_[index].get()->insertIfAbsent(msg.id);
}

bool MessageBucketStore::exists(uint64_t messageId) const {
    std::size_t index = messageId % bucketCount_;
    return buckets_[index].get()->exists(messageId);
}

} // namespace msgpipe::storage
