#include "storage/MessageBucketStore.h"
#include <new>

namespace msgpipe::storage {

MessageBucketStore::MessageBucketStore(std::size_t bucketCount)
    : bucketCount_(bucketCount)
{
    buckets_ = static_cast<MessageBucket**>(::operator new(bucketCount_ * sizeof(MessageBucket*)));
    for (std::size_t i = 0; i < bucketCount_; ++i) {
        buckets_[i] = new MessageBucket();
    }
}

MessageBucketStore::~MessageBucketStore() {
    for (std::size_t i = 0; i < bucketCount_; ++i) {
        delete buckets_[i];
    }
    ::operator delete(buckets_);
}

bool MessageBucketStore::insertIfAbsent(const msgpipe::protocol::Message& msg) {
    std::size_t index = msg.id % bucketCount_;
    return buckets_[index]->insertIfAbsent(msg.id);
}

bool MessageBucketStore::exists(uint64_t messageId) const {
    std::size_t index = messageId % bucketCount_;
    return buckets_[index]->exists(messageId);
}

} // namespace msgpipe::storage
