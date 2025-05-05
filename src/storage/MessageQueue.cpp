#include "storage/MessageQueue.h"
#include <new>
#include <cstring>

#include <iostream>

namespace msgpipe::storage {

MessageQueue::MessageQueue(std::size_t capacity)
    : capacity_(capacity), head_(0), tail_(0)
{
    buffer_ = static_cast<Slot*>(::operator new[](capacity_ * sizeof(Slot)));
    for (std::size_t i = 0; i < capacity_; ++i) {
        new (&buffer_[i]) Slot{false, {}};
    }
}

MessageQueue::~MessageQueue() {
    for (std::size_t i = 0; i < capacity_; ++i) {
        buffer_[i].~Slot();
    }
    ::operator delete[](buffer_);
}

bool MessageQueue::tryPush(const msgpipe::protocol::Message& msg) {
    std::cout << "[QUEUE] tryPush ID: " << msg.id << "\n";

    std::size_t pos = tail_.fetch_add(1, std::memory_order_relaxed) % capacity_;
    Slot& slot = buffer_[pos];

    if (slot.ready.load(std::memory_order_acquire)) {
        return false;
    }
    std::memcpy(&slot.value, &msg, sizeof(msg));
    slot.ready.store(true, std::memory_order_release);
    return true;
}

bool MessageQueue::tryPop(msgpipe::protocol::Message& outMsg) {
    std::size_t pos = head_.load(std::memory_order_relaxed) % capacity_;
    Slot& slot = buffer_[pos];

    if (!slot.ready.load(std::memory_order_acquire)) {
        return false;
    }

    std::memcpy(&outMsg, &slot.value, sizeof(outMsg));
    slot.ready.store(false, std::memory_order_release);
    head_.fetch_add(1, std::memory_order_relaxed);

    std::cout << "[QUEUE] tryPop ID: " << outMsg.id << "\n";
    return true;
}

} // namespace msgpipe::storage
