# msgpipe

## Project Overview

This project implements a high-performance multithreaded message routing system in **C++20**, designed for reliability, minimal memory usage, and strict compliance with custom requirements. It processes UDP messages across two threads, deduplicates them, and routes selected messages via TCP to a third thread. All socket operations are fully **non-blocking**, and the design prioritizes **lock-free** processing and **cross-platform support** (Linux, macOS, Windows).

---

## Features

- Concurrent processing using 2 UDP receivers and 1 TCP sender thread
- Custom deduplication store based on hash buckets
- Lock-free message queue between receiver and sender
- Raw socket programming using platform-native APIs only
- Fully non-blocking I/O for UDP and TCP
- Zero STL containers or algorithms — only raw C++20 and low-level primitives
- Platform support: Linux (epoll), macOS (poll), Windows (WSAPoll / select-compatible)

---

## Optimization Techniques

### Lock-Free Message Queue
Implemented a circular array queue with atomic head/tail pointers. No locks are used for push/pop, ensuring real-time performance.

### Custom Deduplication Store
Replaced STL sets/maps with a fixed-size hash-bucket container. Optimized for rapid lookups and minimal memory allocation.

### Non-Blocking UDP and TCP Sockets
All sockets are initialized in **non-blocking** mode. Platform-specific polling is used:
- **Linux**: `epoll_wait`
- **macOS / Windows**: `poll` (fallback)

This avoids blocking calls and improves thread responsiveness.

### Memory Layout Optimization
Message structures are laid out using raw POD types (`<cstdint>`) to ensure predictable size and alignment. No virtuals or heap allocations used.

### No STL
All core logic — containers, deduplication, queues — is written from scratch using raw pointers and atomics.

### Static Preallocation
Hash buckets and queues are pre-sized at startup, preventing any runtime dynamic allocations or resizing.

### Parallel Input Streams
Two UDP threads independently listen on different ports, allowing horizontal scaling and maximizing throughput.

### Efficient Polling and Sleep Backoff
Polling (epoll/poll) is used with short timeout and thread yields to avoid high CPU usage, while keeping latency low.

---

## Building the Project

### Dependencies

#### Required (All platforms):
- **CMake ≥ 3.16**
- **Python 3.8+** (for helper scripts)
- **Ninja** (recommended, but optional)
- **GoogleTest** (fetched automatically via CMake if `BUILD_TESTING=ON`)

---

### Linux

```bash
sudo apt update
sudo apt install cmake ninja-build g++ python3
git clone https://github.com/yourname/msgpipe.git
cd msgpipe
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/msgpipe
```

---

### macOS

```bash
brew install cmake ninja
git clone https://github.com/yourname/msgpipe.git
cd msgpipe
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/msgpipe
```

---

### Windows (MinGW-w64 + Ninja)

1. Install via Chocolatey:
```bash
choco install mingw cmake ninja -y
refreshenv
```

2. Build the project:
```bash
git clone https://github.com/yourname/msgpipe.git
cd msgpipe
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake --build build
.\build\msgpipe.exe
```

---

## Architecture

- **Main thread**: Initializes and launches all worker threads
- **UDP Input Threads (2x)**: Bind to separate ports, receive messages via `recvfrom`, parse and deduplicate messages
- **Deduplication Store**: Thread-safe, lock-free store that filters messages by `MessageId`
- **MessageQueue**: Lock-free ring buffer used to pass messages between UDP threads and TCP thread
- **TCP Output Thread**: Connects to a remote host and sends selected messages (`MessageData == 10`) via `send()`

---

## Protocol Format

```cpp
struct Message {
  uint16_t MessageSize;
  uint8_t  MessageType;
  uint64_t MessageId;
  uint64_t MessageData;
};
```

Messages are sent via UDP in this binary format. When `MessageData == 10`, they are routed over TCP.

---

## License

MIT License. All code written by **Vitalii Ryzhov** for the Atto Trading Senior C++ test.