#include "workers/TcpOutputWorker.h"
#include "storage/MessageQueue.h"
#include "protocol/Message.h"

#include <gtest/gtest.h>
#include <thread>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <atomic>
#include <cstring>

using namespace msgpipe;

TEST(TcpOutputWorkerTest, SendsMessageToServer) {
    constexpr int kPort = 9060;
    storage::MessageQueue queue(4);
    std::atomic<bool> stop{false};
    std::atomic<bool> received{false};

    // Start mock TCP server
    std::thread serverThread([&]() {
        int serverSock = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(kPort);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        bind(serverSock, (sockaddr*)&addr, sizeof(addr));
        listen(serverSock, 1);

        int clientSock = accept(serverSock, nullptr, nullptr);
        protocol::Message msg{};
        ssize_t len = recv(clientSock, &msg, sizeof(msg), 0);
        if (len == sizeof(msg) && msg.id == 9999 && msg.data == 10) {
            received = true;
        }
        close(clientSock);
        close(serverSock);
    });

    // Fill queue
    protocol::Message msg{sizeof(protocol::Message), 1, 9999, 10};
    queue.tryPush(msg);

    std::thread clientThread([&]() {
        TcpOutputWorker client("127.0.0.1", kPort, queue);
        client.run(stop);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    stop = true;

    clientThread.join();
    serverThread.join();

    EXPECT_TRUE(received.load());
}
