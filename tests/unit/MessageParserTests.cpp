#include "parsers/MessageParser.h"
#include "protocol/Message.h"
#include <gtest/gtest.h>
#include <array>
#include <cstring>

using namespace msgpipe::parsers;
using namespace msgpipe::protocol;

TEST(MessageParserTest, ParseValidBuffer) {
    MessageParser parser;
    Message input{sizeof(Message), 2, 555, 10};
    std::array<std::byte, sizeof(Message)> buffer;
    std::memcpy(buffer.data(), &input, sizeof(Message));

    auto parsed = parser.parse(buffer.data(), buffer.size());
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->id, 555);
    EXPECT_EQ(parsed->data, 10);
    EXPECT_EQ(parsed->type, 2);
}

TEST(MessageParserTest, ParseTooShortBuffer) {
    MessageParser parser;
    std::array<std::byte, sizeof(Message) - 2> buffer{};
    auto parsed = parser.parse(buffer.data(), buffer.size());
    EXPECT_FALSE(parsed.has_value());
}
