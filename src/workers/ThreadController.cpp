#include "workers/ThreadController.h"
#include <iostream>

namespace msgpipe::workers {

ThreadController::ThreadController(int udpPort1,
                                   int udpPort2,
                                   const std::string& tcpHost,
                                   int tcpPort,
                                   msgpipe::storage::MessageBucketStore& store,
                                   msgpipe::storage::MessageQueue& queue)
    : store_(store), queue_(queue)
{
    udp1_ = std::make_unique<UdpInputWorker>(udpPort1, store_, queue_);
    udp2_ = std::make_unique<UdpInputWorker>(udpPort2, store_, queue_);
    tcp_  = std::make_unique<TcpOutputWorker>(tcpHost, tcpPort, queue_);
}

ThreadController::~ThreadController() {
    stop();
}

void ThreadController::start() {
    tcp_->connect();

#if defined(__APPLE__)
    threads_[0] = std::thread([this]() { udp1_->run(stop_); });
    threads_[1] = std::thread([this]() { udp2_->run(stop_); });
    threads_[2] = std::thread([this]() { tcp_->run(stop_); });
#else
    threads_[0] = std::jthread([this](std::stop_token st) { udp1_->run(st); });
    threads_[1] = std::jthread([this](std::stop_token st) { udp2_->run(st); });
    threads_[2] = std::jthread([this](std::stop_token st) { tcp_->run(st); });
#endif

    threadCount_ = 3;
    std::cout << "[msgpipe] All workers started. Press Ctrl+C to exit.\n";
}

void ThreadController::stop() {
#if defined(__APPLE__)
    stop_ = true;
    for (int i = 0; i < threadCount_; ++i) {
        if (threads_[i].joinable()) {
            threads_[i].detach();
        }
    }
#else
    for (int i = 0; i < threadCount_; ++i) {
        threads_[i].request_stop();
    }
#endif
}


} // namespace msgpipe::workers
