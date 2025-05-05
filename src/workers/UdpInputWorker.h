/**
 * @file UdpInputWorker.h
 * @brief Defines the UdpInputWorker class responsible for receiving messages over UDP.
 */
#pragma once

#include "protocol/Message.h"
#include "parsers/MessageParser.h"
#include "storage/MessageBucketStore.h"
#include "storage/MessageQueue.h"

#include <cstddef>
#include <atomic>

#if defined(_WIN32)
    #define NOMINMAX
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    using socket_t = int;
#endif

namespace msgpipe::workers {

/**
 * @class UdpInputWorker
 * @brief Receives UDP packets, parses messages and pushes relevant ones to queue.
 *
 * This class binds to a given UDP port, listens non-blockingly, and parses any received packets
 * into structured Message objects using MessageParser. Messages with `data == 10` are forwarded to the queue.
 */
class UdpInputWorker {
public:
    /**
     * @brief Constructs and binds the worker to a UDP port.
     * @param port The port to bind and listen on.
     * @param store Reference to the deduplication store.
     * @param queue Queue to push valid messages into.
     */
    UdpInputWorker(int port,
                   msgpipe::storage::MessageBucketStore& store,
                   msgpipe::storage::MessageQueue& queue);

    ~UdpInputWorker();

    /**
     * @brief Blocking execution loop. Receives and parses UDP packets until stop flag is set.
     * @param stop Atomic flag indicating external shutdown.
     */
    void run(std::atomic<bool>& stop);

private:
    socket_t sock_;  ///< UDP socket handle (cross-platform)
#if !defined(_WIN32)
    sockaddr_in addr_; ///< Bound socket address (UNIX only)
#endif
    msgpipe::parsers::MessageParser parser_; ///< Parses raw bytes into Message structs
    msgpipe::storage::MessageBucketStore& store_; ///< Deduplication store
    msgpipe::storage::MessageQueue& queue_; ///< Queue for forwarding parsed messages
};
} // namespace msgpipe::workers