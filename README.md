# msgpipe — Low-latency UDP/TCP Dispatcher

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Run

```bash
./msgpipe
```

## Description

High-performance message receiver and dispatcher:
- 2 threads receive UDP messages
- 1 thread sends via TCP if MessageData == 10
- Lock-free message store and queue
- STL-free design
- Clean modular architecture (SOLID principles)

## Structure

- `common/` — Message definitions and interfaces
- `network/` — UDP and TCP logic, socket utilities
- `storage/` — HashMap and queue logic
- `threading/` — Thread lifecycle and shutdown management
