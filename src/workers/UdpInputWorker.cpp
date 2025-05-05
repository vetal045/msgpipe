#include "workers/UdpInputWorker.h"
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

#if defined(__linux__)
    #include <sys/epoll.h>
#elif defined(__APPLE__) || defined(_WIN32)
    #include <poll.h>
#endif

namespace {

constexpr std::size_t kMaxPacketSize = sizeof(msgpipe::protocol::Message);

int makeSocketNonBlocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

} // namespace

UdpInputWorker::UdpInputWorker(
    int port,
    msgpipe::storage::MessageBucketStore& store,
    msgpipe::storage::MessageQueue& queue)
    : store_(store), queue_(queue)
{
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ < 0) {
        perror("socket");
        std::exit(1);
    }

    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = INADDR_ANY;
    addr_.sin_port = htons(port);

    if (bind(sock_, reinterpret_cast<sockaddr*>(&addr_), sizeof(addr_)) < 0) {
        perror("bind");
        std::exit(1);
    }

    if (makeSocketNonBlocking(sock_) < 0) {
        perror("fcntl");
        std::exit(1);
    }
}

UdpInputWorker::~UdpInputWorker() {
    close(sock_);
}

void UdpInputWorker::run(std::atomic<bool>& stop) {
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

    while (!stop.load()) {
        int nfds = epoll_wait(epollFd, events, 1, 100);
        if (nfds <= 0) continue;

        ssize_t received = recvfrom(sock_, buffer, kMaxPacketSize, 0, nullptr, nullptr);
        if (received <= 0) continue;

        auto parsed = parser_.parse(buffer, static_cast<std::size_t>(received));
        if (!parsed.has_value()) continue;

        const auto& msg = parsed.value();
        if (store_.insertIfAbsent(msg)) {
            if (msg.data == 10) {
                queue_.tryPush(msg);
            }
        }
    }

    close(epollFd);
#else
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
        if (store_.insertIfAbsent(msg)) {
            if (msg.data == 10) {
                queue_.tryPush(msg);
            }
        }
    }
#endif
}
