#include "workers/TcpOutputWorker.h"

#if defined(_WIN32)
    #define NOMINMAX
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <cstring>
#endif

#include <thread>
#include <iostream>

namespace {

int connectWithRetry(const std::string& host, int port, int max_ms = 10000) {
#if defined(_WIN32)
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) throw std::runtime_error("Failed to create TCP socket");
#else
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) throw std::runtime_error("Failed to create TCP socket");
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
#if defined(_WIN32)
        closesocket(sock);
#else
        close(sock);
#endif
        throw std::runtime_error("Invalid address: " + host);
    }

    auto start = std::chrono::steady_clock::now();

    while (true) {
        int result = connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        if (result == 0) {
            return sock;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() >= max_ms) {
#if defined(_WIN32)
            closesocket(sock);
#else
            close(sock);
#endif
            throw std::runtime_error("Connection timeout to " + host + ":" + std::to_string(port));
        }
    }
}
} // namespace

namespace msgpipe::workers {
TcpOutputWorker::TcpOutputWorker(const std::string& host, int port, msgpipe::storage::MessageQueue& queue)
    : host_(host), port_(port), queue_(queue)
{
#if defined(_WIN32)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
    }
#endif
}

void TcpOutputWorker::connect() {
    sock_ = connectWithRetry(host_, port_, 10000);
    if (sock_ < 0) {
        throw std::runtime_error("TCP connection failed");
    }
}

TcpOutputWorker::~TcpOutputWorker() {
#if defined(_WIN32)
    closesocket(sock_);
    WSACleanup();
#else
    close(sock_);
#endif
}

void TcpOutputWorker::run(std::atomic<bool>& stop) {
    msgpipe::protocol::Message msg;

    while (!stop.load()) {
        if (!queue_.tryPop(msg)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

#if defined(_WIN32)
        int sent = send(sock_, reinterpret_cast<const char*>(&msg), sizeof(msg), 0);
#else
        ssize_t sent = send(sock_, &msg, sizeof(msg), 0);
#endif

        if (sent != sizeof(msg)) {
            throw std::runtime_error("[TCP] send failed");
        }
    }
}
} // namespace msgpipe::workers