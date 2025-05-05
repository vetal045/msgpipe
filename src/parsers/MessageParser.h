#pragma once

#include "protocol/Message.h"

#include <optional>

namespace msgpipe::parsers {

/**
 * @brief Default implementation of IMessageParser for raw binary protocol.
 */
class MessageParser {
public:
    std::optional<msgpipe::protocol::Message> parse(const std::byte* data, std::size_t length) const;
};

} // namespace msgpipe::parsers
