#include "workers/UdpInputWorker.h"
#include "storage/MessageBucketStore.h"
#include "storage/MessageQueue.h"
#include "protocol/Message.h"

#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>

#if defined(_WIN32)
    #define NOMINMAX
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

using namespace msgpipe;

TEST(UdpInputWorkerTest, ReceivesMessageAndPushesToQueue) {
    constexpr int kPort = 9050;
    storage::MessageBucketStore store(64);
    storage::MessageQueue queue(8);

#if defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

#if defined(__APPLE__)
    std::atomic<bool> stop{false};
    std::thread workerThread([&]() {
        msgpipe::workers::UdpInputWorker worker(kPort, store, queue);
        worker.run(stop);
    });
#else
    std::jthread workerThread([&](std::stop_token st) {
        msgpipe::workers::UdpInputWorker worker(kPort, store, queue);
        worker.run(st);
    });
#endif

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

#if defined(_WIN32)
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#else
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(kPort);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    protocol::Message msg{sizeof(protocol::Message), 1, 12345, 10};
#if defined(_WIN32)
    sendto(sock, reinterpret_cast<const char*>(&msg), sizeof(msg), 0,
           reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    closesocket(sock);
#else
    sendto(sock, &msg, sizeof(msg), 0,
           reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    close(sock);
#endif

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

#if defined(__APPLE__)
    stop = true;
    workerThread.join();
#else
    workerThread.request_stop();
    workerThread.join();
#endif

#if defined(_WIN32)
    WSACleanup();
#endif

    protocol::Message out{};
    EXPECT_TRUE(queue.tryPop(out));
    EXPECT_EQ(out.id, 12345);
    EXPECT_EQ(out.data, 10);
}
