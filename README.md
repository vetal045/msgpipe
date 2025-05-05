# msgpipe

## Project Overview

This project implements a high-performance multithreaded message routing system in **C++20**, designed for reliability, minimal memory usage, and strict compliance with custom low-level socket requirements. It processes UDP messages across two threads, deduplicates them, and routes selected messages via TCP to a third thread. All socket operations are fully **non-blocking**, and the design prioritizes **lock-free processing** and **cross-platform compatibility** (Linux, macOS, Windows).

---

## Features

- Concurrent processing using 2 UDP receivers and 1 TCP sender thread
- Raw binary message protocol (fixed layout)
- Platform-agnostic network layer with conditional compilation
- Lock-free message queue with atomic operations
- Custom bucket-based deduplication store
- Fully non-blocking I/O using epoll/poll/select/WSAPoll
- Clean separation of responsibilities: input, output, deduplication, control
- Unit-tested core components
- Complete cross-platform support (Linux, macOS, Windows)

---

## Project Structure

| Path | Description |
|------|-------------|
| `src/protocol/` | Binary message format definition |
| `src/parsers/`  | Message parsing logic |
| `src/storage/`  | Lock-free message queue and deduplication store |
| `src/workers/`  | UDP input and TCP output thread implementations |
| `src/main.cpp`  | Entry point launching threads and initializing system |
| `tests/unit/`   | Unit tests for MessageQueue, MessageStore, Parser |
| `tests/integration/` | TCP and UDP thread integration tests |
| `Doxyfile`      | Doxygen configuration for code documentation |
| `send_udp.py`   | Optional UDP client for testing |

---

## Architecture Overview

- **UDP Threads (2x):**
  - Each binds to a separate port and listens using `recvfrom()` in non-blocking mode.
  - Uses `epoll` (Linux), `poll` (macOS), or `WSAPoll/select` (Windows).

- **MessageBucketStore:**
  - Lock-free deduplication using pre-allocated array of buckets.
  - Based on `MessageId`, skips already-seen messages.

- **MessageQueue:**
  - Circular ring buffer with atomic `head`/`tail` pointers.
  - Lock-free, single-producer single-consumer design.

- **TCP Sender Thread:**
  - Establishes TCP connection on startup.
  - Sends messages with `MessageData == 10` over `send()`.

- **ThreadController:**
  - Starts and stops all threads cleanly.
  - Uses `std::jthread` where available (Linux/Windows), fallback to `std::thread` on macOS.

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

All messages follow this layout in binary form. The message is routed to the TCP thread only if `MessageData == 10` and it has not been seen before.

---

## Optimization Techniques

### Lock-Free Message Queue
The queue is implemented as a ring buffer using atomics, with wrap-around indexing. Push and pop do not use any locks and are fast even under high throughput.

### Custom Deduplication Store
The deduplication store is implemented as an array of raw pointer-based buckets indexed by a hash of `MessageId`. It supports fast lookup and insertion, and never resizes at runtime.

### Non-Blocking Socket I/O
All socket descriptors are explicitly configured as non-blocking. On Linux, `epoll` is used for efficient event waiting. On macOS, `poll` is used. On Windows, either `poll` or `select`-compatible behavior is used via `WSAPoll`.

### Platform Abstraction
Socket initialization, polling, and cleanup are wrapped in minimal abstractions, with conditional compilation per platform. Platform differences are hidden behind clean interfaces.

### Memory Layout and Allocation
All message and container structures are tightly packed. There are no heap allocations or STL usage in core processing. Pre-allocated buffers ensure real-time responsiveness.

### Zero STL Containers or Algorithms
All containers (queue, store) are implemented from scratch to comply with the restriction on using STL.

### Multi-threaded Processing
All core components operate safely in multi-threaded contexts using only atomic variables and platform synchronization primitives.

---

## Building the Project

### Dependencies (All Platforms)

- **CMake** ≥ 3.16
- **Python 3.8+** (optional, for scripts)
- **Ninja** (recommended)
- **GoogleTest** (auto-fetched via CMake if `BUILD_TESTING=ON`)
- **Doxygen** (for documentation generation)

---

### Linux

```bash
sudo apt update
sudo apt install cmake g++ ninja-build python3 doxygen
git clone https://github.com/yourname/msgpipe.git
cd msgpipe
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/msgpipe
```

---

### macOS

```bash
brew install cmake ninja doxygen
git clone https://github.com/yourname/msgpipe.git
cd msgpipe
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/msgpipe
```

---

### Windows (MinGW-w64 + Ninja + Doxygen)

Install with Chocolatey (Run as Admin PowerShell):

```bash
choco install mingw cmake ninja doxygen.install -y
refreshenv
```

Then build:

```bash
git clone https://github.com/yourname/msgpipe.git
cd msgpipe
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake --build build
.\build\msgpipe.exe
```

> If `doxygen` is not found, ensure `C:\Program Files\doxygenin` is in your PATH or run manually.

---

## Unit Tests

All unit and integration tests are written using **GoogleTest**.

### Build and Run Tests

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build
./build/msgpipe_unit_tests         # Unit tests
./build/msgpipe_integration_tests  # Integration tests
```

---

## Doxygen Documentation

### Generate HTML docs:

```bash
cd build
doxygen ../Doxyfile
```

Output will be available in `build/docs/html/index.html`

---

## License

MIT License. All code written by **Vitalii Ryzhov** for the Atto Trading Senior C++ test.