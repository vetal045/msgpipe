#include "workers/UdpInputWorker.h"
#include "storage/MessageBucketStore.h"
#include "storage/MessageQueue.h"
#include "protocol/Message.h"

#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

using namespace msgpipe;

TEST(UdpInputWorkerTest, ReceivesMessageAndPushesToQueue) {
    constexpr int kPort = 9050;
    storage::MessageBucketStore store(64);
    storage::MessageQueue queue(8);
    std::atomic<bool> stop{false};

    std::thread workerThread([&]() {
        UdpInputWorker worker(kPort, store, queue);
        worker.run(stop);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // let worker bind

    // Send UDP message
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(kPort);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    protocol::Message msg{sizeof(protocol::Message), 1, 12345, 10};
    sendto(sock, &msg, sizeof(msg), 0, (sockaddr*)&addr, sizeof(addr));
    close(sock);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    stop = true;
    workerThread.join();

    protocol::Message out{};
    EXPECT_TRUE(queue.tryPop(out));
    EXPECT_EQ(out.id, 12345);
    EXPECT_EQ(out.data, 10);
}
