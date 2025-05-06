#include "workers/TcpOutputWorker.h"
#include "storage/MessageQueue.h"
#include "protocol/Message.h"

#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include <future>
#include <cstring>
#include <iostream>

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

TEST(TcpOutputWorkerTest, SendsMessageToServer) {
    constexpr int kPort = 9060;
    storage::MessageQueue queue(4);
    std::atomic<bool> received{false};

    std::promise<void> serverReady;
    std::shared_future<void> serverReadyFuture = serverReady.get_future().share();

#if defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    // TCP mock server
    std::thread serverThread([&]() {
#if defined(_WIN32)
        SOCKET serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
        int serverSock = socket(AF_INET, SOCK_STREAM, 0);
#endif
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(kPort);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        if (bind(serverSock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            std::cerr << "[Server] bind() failed\n";
            return;
        }

        if (listen(serverSock, 1) < 0) {
            std::cerr << "[Server] listen() failed\n";
            return;
        }

        serverReady.set_value(); // signal that server is ready

#if defined(_WIN32)
        SOCKET clientSock = accept(serverSock, nullptr, nullptr);
#else
        int clientSock = accept(serverSock, nullptr, nullptr);
#endif

        if (clientSock < 0) {
            std::cerr << "[Server] accept() failed\n";
            return;
        }

        protocol::Message msg{};
#if defined(_WIN32)
        int len = recv(clientSock, reinterpret_cast<char*>(&msg), sizeof(msg), 0);
#else
        ssize_t len = recv(clientSock, &msg, sizeof(msg), 0);
#endif

        if (len == sizeof(msg) && msg.id == 9999 && msg.data == 10) {
            received = true;
        }

#if defined(_WIN32)
        closesocket(clientSock);
        closesocket(serverSock);
#else
        close(clientSock);
        close(serverSock);
#endif
    });

    // prepare message
    protocol::Message msg{sizeof(protocol::Message), 1, 9999, 10};
    queue.tryPush(msg);

#if defined(__APPLE__)
    std::atomic<bool> stop{false};
    std::thread clientThread([&]() {
        try {
            serverReadyFuture.wait(); // wait for server to be ready

            auto client = std::make_unique<msgpipe::workers::TcpOutputWorker>("127.0.0.1", kPort, queue);
            client->connect();
            client->run(stop);
        } catch (const std::exception& ex) {
            std::cerr << "[ClientThread] Exception: " << ex.what() << "\n";
        } catch (...) {
            std::cerr << "[ClientThread] Unknown exception\n";
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    stop = true;
    clientThread.join();
#else
    std::jthread clientThread([&](std::stop_token st) {
        try {
            serverReadyFuture.wait(); // wait for server to be ready

            auto client = std::make_unique<msgpipe::workers::TcpOutputWorker>("127.0.0.1", kPort, queue);
            client->connect();
            client->run(st);
        } catch (const std::exception& ex) {
            std::cerr << "[ClientThread] Exception: " << ex.what() << "\n";
        } catch (...) {
            std::cerr << "[ClientThread] Unknown exception\n";
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    clientThread.request_stop();
    clientThread.join();
#endif

    serverThread.join();

#if defined(_WIN32)
    WSACleanup();
#endif

    EXPECT_TRUE(received.load());
}
