# tests/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(msgpipe_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

include(GoogleTest)

# Add test sources
set(TEST_SOURCES
    unit/test_message_queue.cpp
    unit/test_message_bucket.cpp
    unit/test_message_bucket_store.cpp
    unit/test_message_parser.cpp
)

add_executable(msgpipe_tests ${TEST_SOURCES})
target_include_directories(msgpipe_tests PRIVATE ../src)
target_link_libraries(msgpipe_tests gtest_main)

# Link platform-specific if needed
if (WIN32)
    target_link_libraries(msgpipe_tests ws2_32)
endif()

gtest_discover_tests(msgpipe_tests)
