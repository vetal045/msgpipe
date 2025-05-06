#pragma once

#include <utility>
#include <stdexcept>

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <unistd.h>
#endif

namespace msgpipe::workers {

/**
 * @class SocketFd
 * @brief Cross-platform RAII wrapper for POSIX/Windows socket descriptors.
 */
class SocketFd {
public:
    explicit SocketFd(int fd)
        : fd_(fd)
    {
    }

    ~SocketFd() noexcept {
        if (fd_ >= 0) {
    #if defined(_WIN32)
            ::closesocket(fd_);
    #else
            ::close(fd_);
    #endif
        }
    }
    

    SocketFd(SocketFd&& other) noexcept
        : fd_(std::exchange(other.fd_, -1)) {}

    SocketFd& operator=(SocketFd&& other) noexcept {
        if (this != &other) {
#if defined(_WIN32)
            if (fd_ >= 0) ::closesocket(fd_);
#else
            if (fd_ >= 0) ::close(fd_);
#endif
            fd_ = std::exchange(other.fd_, -1);
        }
        return *this;
    }

    SocketFd(const SocketFd&) = delete;
    SocketFd& operator=(const SocketFd&) = delete;

    int get() const { return fd_; }
    bool valid() const { return fd_ >= 0; }

private:
    int fd_;
};

} // namespace msgpipe::workers
