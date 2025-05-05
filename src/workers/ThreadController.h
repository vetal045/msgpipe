#pragma once

#include "workers/UdpInputWorker.h"
#include "workers/TcpOutputWorker.h"

#include "storage/MessageQueue.h"
#include "storage/MessageBucketStore.h"

#include <memory>
#include <string>

#if defined(__APPLE__)
    #include <thread>
#else
    #include <thread>
    #include <stop_token>
#endif

namespace msgpipe::workers {

/**
 * @class ThreadController
 * @brief Manages the lifecycle of all worker threads (UDP input, TCP output).
 *
 * Starts each worker in a dedicated thread and ensures proper cleanup on shutdown.
 * Uses std::jthread with stop_token where supported (Linux/Windows), otherwise std::thread.
 */
class ThreadController {
public:
    /**
     * @brief Constructs the thread controller with necessary worker dependencies.
     * @param udpPort1 First UDP listen port
     * @param udpPort2 Second UDP listen port
     * @param tcpHost Destination TCP host
     * @param tcpPort Destination TCP port
     * @param store Shared deduplication store
     * @param queue Shared lock-free message queue
     */
    ThreadController(int udpPort1,
                     int udpPort2,
                     const std::string& tcpHost,
                     int tcpPort,
                     msgpipe::storage::MessageBucketStore& store,
                     msgpipe::storage::MessageQueue& queue);

    /// @brief Gracefully joins or detaches all worker threads
    ~ThreadController();

    /// @brief Starts all worker threads
    void start();

    /// @brief Initiates shutdown (no-op on macOS, stop_token on others)
    void stop();

private:
#if defined(__APPLE__)
    std::thread threads_[3];
#else
    std::jthread threads_[3];
#endif

    int threadCount_ = 0;

    std::unique_ptr<UdpInputWorker> udp1_;
    std::unique_ptr<UdpInputWorker> udp2_;
    std::unique_ptr<TcpOutputWorker> tcp_;

    msgpipe::storage::MessageBucketStore& store_;
    msgpipe::storage::MessageQueue& queue_;
    std::atomic<bool> stop_{false};
};

} // namespace msgpipe::workers
