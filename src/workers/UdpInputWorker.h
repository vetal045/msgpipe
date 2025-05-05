#pragma once

#include "protocol/Message.h"
#include "parsers/MessageParser.h"
#include "storage/MessageBucketStore.h"
#include "storage/MessageQueue.h"

#include <cstddef>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

/**
 * @class UdpInputWorker
 * @brief Thread-executed component that receives and processes messages over UDP.
 */
class UdpInputWorker {
public:
    /**
     * @brief Constructs the worker with references to storage and queue.
     * @param port UDP port to bind to
     * @param store Message deduplication store
     * @param queue Queue for forwarding messages with data == 10
     */
    UdpInputWorker(int port,
                   msgpipe::storage::MessageBucketStore& store,
                   msgpipe::storage::MessageQueue& queue);

    ~UdpInputWorker();

    /**
     * @brief Starts the blocking receive loop. Intended to be run in a thread.
     */
    void run(std::atomic<bool>& stop);

private:
    int sock_;
    sockaddr_in addr_;
    msgpipe::parsers::MessageParser parser_;
    msgpipe::storage::MessageBucketStore& store_;
    msgpipe::storage::MessageQueue& queue_;
};
