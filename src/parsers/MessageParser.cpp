#include "parsers/MessageParser.h"
#include <cstring>

namespace msgpipe::parsers {

std::optional<msgpipe::protocol::Message> MessageParser::parse(const std::byte* data, std::size_t length) const {
    if (length < msgpipe::protocol::ExpectedMessageSize) {
        return std::nullopt;
    }

    msgpipe::protocol::Message msg;
    std::memcpy(&msg, data, msgpipe::protocol::ExpectedMessageSize);
    return msg;
}

} // namespace msgpipe::parsers
