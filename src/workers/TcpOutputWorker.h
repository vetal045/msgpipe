#pragma once

#include "storage/MessageQueue.h"
#include "protocol/Message.h"
#include "workers/SocketFd.h"

#include <string>
#include <stop_token>

namespace msgpipe::workers {
/**
 * @class TcpOutputWorker
 * @brief Reads messages from the queue and sends them to a TCP destination.
 *
 * Designed to run in a dedicated thread. Connects once on startup,
 * then loops forever, draining messages from the queue and sending them out.
 */
class TcpOutputWorker {
public:
    /**
     * @brief Constructs a worker for sending messages over TCP.
     * @param host Destination IP address (IPv4)
     * @param port Destination port
     * @param queue Queue from which to consume messages
     */
    TcpOutputWorker(const std::string& host,
                    int port,
                    msgpipe::storage::MessageQueue& queue);

    ~TcpOutputWorker();

    /**
     * @brief Starts the blocking loop that sends messages from queue over TCP.
     */
#if defined(__APPLE__)
    void run(std::atomic<bool>& stop);
#else
    void run(std::stop_token stopToken);
#endif

    /// @brief Must be called once before run().
    void connect();

private:
    SocketFd sock_{-1};
    std::string host_;
    int port_;
    msgpipe::storage::MessageQueue& queue_;
};
} // namespace msgpipe::workers