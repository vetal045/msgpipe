#include "workers/UdpInputWorker.h"

#include <iostream>
#include <cstring>
#include <thread>

#if defined(__linux__)
    #include <fcntl.h>
    #include <sys/epoll.h>
#elif defined(__APPLE__)
    #include <poll.h>
    #include <fcntl.h>
#elif defined(_WIN32)
    #include <ws2tcpip.h>
#endif

namespace {
constexpr std::size_t kMaxPacketSize = sizeof(msgpipe::protocol::Message);

#if !defined(_WIN32)
int makeSocketNonBlocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}
#endif
} // namespace

namespace msgpipe::workers {
UdpInputWorker::UdpInputWorker(
    int port,
    msgpipe::storage::MessageBucketStore& store,
    msgpipe::storage::MessageQueue& queue)
    : store_(store), queue_(queue)
{
#if defined(_WIN32)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET) {
        throw std::runtime_error("UDP socket creation failed");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        throw std::runtime_error("UDP bind failed");
    }
#else
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ < 0) {
        throw std::runtime_error("socket() failed");
    }

    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = INADDR_ANY;
    addr_.sin_port = htons(port);

    if (bind(sock_, reinterpret_cast<sockaddr*>(&addr_), sizeof(addr_)) < 0) {
        throw std::runtime_error("UDP bind failed: " + std::string(strerror(errno)));
    }

    makeSocketNonBlocking(sock_);
#endif
}

UdpInputWorker::~UdpInputWorker() {
#if defined(_WIN32)
    closesocket(sock_);
    WSACleanup();
#else
    close(sock_);
#endif
}

#if defined(__APPLE__)
void UdpInputWorker::run(std::atomic<bool>& stop)
#else
void UdpInputWorker::run(std::stop_token stopToken)
#endif
{
    std::byte buffer[kMaxPacketSize];

#if defined(__linux__)
    int epollFd = epoll_create1(0);
    if (epollFd < 0) {
        perror("epoll_create1");
        return;
    }

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = sock_;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, sock_, &ev);
    epoll_event events[1];

#if defined(__APPLE__)
    while (!stop.load())
#else
    while (!stopToken.stop_requested())
#endif
    {
        int nfds = epoll_wait(epollFd, events, 1, 100);
        if (nfds <= 0) continue;

        ssize_t received = recvfrom(sock_, buffer, kMaxPacketSize, 0, nullptr, nullptr);
        if (received <= 0) continue;

        auto parsed = parser_.parse(buffer, static_cast<std::size_t>(received));
        if (!parsed.has_value()) continue;

        const auto& msg = parsed.value();
        if (store_.insertIfAbsent(msg) && msg.data == 10) {
            queue_.tryPush(msg);
        }
    }

    close(epollFd);

#elif defined(__APPLE__)
    pollfd pfd{};
    pfd.fd = sock_;
    pfd.events = POLLIN;

    while (!stop.load()) {
        int rc = poll(&pfd, 1, 100);
        if (rc <= 0) continue;

        ssize_t received = recvfrom(sock_, buffer, kMaxPacketSize, 0, nullptr, nullptr);
        if (received <= 0) continue;

        auto parsed = parser_.parse(buffer, static_cast<std::size_t>(received));
        if (!parsed.has_value()) continue;

        const auto& msg = parsed.value();
        if (store_.insertIfAbsent(msg) && msg.data == 10) {
            queue_.tryPush(msg);
        }
    }

#elif defined(_WIN32)
    while (!stop.load()) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock_, &readfds);

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        int rc = select(0, &readfds, nullptr, nullptr, &timeout);
        if (rc <= 0) continue;

        int received = recvfrom(sock_, reinterpret_cast<char*>(buffer), static_cast<int>(kMaxPacketSize), 0, nullptr, nullptr);
        if (received <= 0) continue;

        auto parsed = parser_.parse(buffer, static_cast<std::size_t>(received));
        if (!parsed.has_value()) continue;

        const auto& msg = parsed.value();
        if (store_.insertIfAbsent(msg) && msg.data == 10) {
            queue_.tryPush(msg);
        }
    }
#endif
}

} // namespace msgpipe::workers