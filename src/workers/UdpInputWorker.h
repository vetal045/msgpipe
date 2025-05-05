#pragma once

#include "protocol/Message.h"
#include "parsers/MessageParser.h"
#include "storage/MessageBucketStore.h"
#include "storage/MessageQueue.h"

#include <cstddef>
#include <atomic>

#if defined(_WIN32)
    #define NOMINMAX
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    using socket_t = int;
#endif

class UdpInputWorker {
public:
    UdpInputWorker(int port,
                   msgpipe::storage::MessageBucketStore& store,
                   msgpipe::storage::MessageQueue& queue);
    ~UdpInputWorker();

    void run(std::atomic<bool>& stop);

private:
    socket_t sock_;
#if !defined(_WIN32)
    sockaddr_in addr_;
#endif
    msgpipe::parsers::MessageParser parser_;
    msgpipe::storage::MessageBucketStore& store_;
    msgpipe::storage::MessageQueue& queue_;
};
