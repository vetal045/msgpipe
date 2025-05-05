#include "workers/ThreadController.h"
#include "storage/MessageBucketStore.h"
#include "storage/MessageQueue.h"

#include <csignal>
#include <cstdlib>
#include <iostream>

#if defined(__APPLE__)
    #include <unistd.h> // for pause()
#endif

int main() {
    // Configuration
    constexpr int kUdpPort1 = 5000;
    constexpr int kUdpPort2 = 5001;
    constexpr const char* kTcpHost = "127.0.0.1";
    constexpr int kTcpPort = 9000;
    constexpr std::size_t kBucketCount = 1024;
    constexpr std::size_t kQueueCapacity = 65536;

    // Init core components
    msgpipe::storage::MessageBucketStore store(kBucketCount);
    msgpipe::storage::MessageQueue queue(kQueueCapacity);

    // Launch controller
    msgpipe::workers::ThreadController controller(
        kUdpPort1, kUdpPort2, kTcpHost, kTcpPort, store, queue
    );

    controller.start();

    // Handle Ctrl+C
    std::signal(SIGINT, [](int) {
        std::cout << "\n[msgpipe] Interrupt received. Shutting down...\n";
        std::exit(0);
    });

#if defined(__APPLE__)
    pause();
#else
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
#endif

    return 0;
}
