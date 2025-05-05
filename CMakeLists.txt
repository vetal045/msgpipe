cmake_minimum_required(VERSION 3.20)
project(msgpipe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------
# GoogleTest
# --------------------------------------
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# --------------------------------------
# Sources
# --------------------------------------
set(PROTOCOL
    src/protocol/Message.h
)

set(PARSERS
    src/parsers/MessageParser.h
    src/parsers/MessageParser.cpp
)

set(STORAGE
    src/storage/MessageBucket.h
    src/storage/MessageBucket.cpp
    src/storage/MessageBucketStore.h
    src/storage/MessageBucketStore.cpp
    src/storage/MessageQueue.h
    src/storage/MessageQueue.cpp
)

set(WORKERS
    src/workers/UdpInputWorker.h
    src/workers/UdpInputWorker.cpp
    src/workers/TcpOutputWorker.h
    src/workers/TcpOutputWorker.cpp
    src/workers/ThreadController.h
    src/workers/ThreadController.cpp
)

set(MAIN
    src/main.cpp
)

# --------------------------------------
# Main target
# --------------------------------------
add_executable(msgpipe
    ${MAIN}
    ${PROTOCOL}
    ${PARSERS}
    ${STORAGE}
    ${WORKERS}
)

target_include_directories(msgpipe PRIVATE src)

# --------------------------------------
# Unit test target (без сокетов)
# --------------------------------------
add_executable(msgpipe_unit_tests
    tests/unit/MessageQueueTests.cpp
    tests/unit/MessageBucketTests.cpp
    tests/unit/MessageStoreTests.cpp
    tests/unit/MessageParserTests.cpp

    ${PROTOCOL}
    ${PARSERS}
    ${STORAGE}
)

target_include_directories(msgpipe_unit_tests PRIVATE src)
target_link_libraries(msgpipe_unit_tests gtest_main)

gtest_discover_tests(msgpipe_unit_tests)

# --------------------------------------
# Integration test target (с сокетами)
# --------------------------------------
add_executable(msgpipe_integration_tests
    tests/integration/UdpInputWorketTests.cpp
    tests/integration/TcpOutputWorkerTests.cpp

    ${PROTOCOL}
    ${PARSERS}
    ${STORAGE}
    ${WORKERS}
)

target_include_directories(msgpipe_integration_tests PRIVATE src)
target_link_libraries(msgpipe_integration_tests gtest_main)

gtest_discover_tests(msgpipe_integration_tests)

# --------------------------------------
# Platform socket support
# --------------------------------------
if (WIN32)
    target_link_libraries(msgpipe ws2_32)
    target_link_libraries(msgpipe_unit_tests ws2_32)
    target_link_libraries(msgpipe_integration_tests ws2_32)
endif()
