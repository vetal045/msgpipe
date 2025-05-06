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

msgpipe::workers::SocketFd connectWithRetry(const std::string& host, int port, int timeoutMs) {
    using namespace std::chrono_literals;

    auto start = std::chrono::steady_clock::now();
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) {
        throw std::runtime_error("connectWithRetry: invalid address: " + host);
    }

    while (true) {
        int rawSock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (rawSock < 0) {
            throw std::runtime_error("connectWithRetry: socket creation failed");
        }

        msgpipe::workers::SocketFd sock(rawSock); // RAII!

        if (::connect(sock.get(), reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == 0) {
            return sock;
        }

        // Connection failed, check timeout
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > timeoutMs) {
            throw std::runtime_error("connectWithRetry: timed out trying to connect");
        }

        std::this_thread::sleep_for(100ms);
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
        throw std::runtime_error("WSAStartup failed");
    }
#endif
}

void TcpOutputWorker::connect() {
    sock_ = connectWithRetry(host_, port_, 3000);
}

TcpOutputWorker::~TcpOutputWorker() {
#if defined(_WIN32)
    WSACleanup();
#endif
}

#if defined(__APPLE__)
void TcpOutputWorker::run(std::atomic<bool>& stop)
#else
void TcpOutputWorker::run(std::stop_token stopToken)
#endif
{
    msgpipe::protocol::Message msg;

#if defined(__APPLE__)
    while (!stop.load()) {
#else
    while (!stopToken.stop_requested()) {
#endif
        if (!queue_.tryPop(msg)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        ssize_t sent = ::send(sock_.get(), &msg, sizeof(msg), 0);
        if (sent != sizeof(msg)) {
            throw std::runtime_error("[TCP] send failed");
        }
    }
}

} // namespace msgpipe::workers