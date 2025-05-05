/**
 * @file MessageParser.h
 * @brief Parser for transforming byte arrays into Message structs.
 */
#pragma once

#include "protocol/Message.h"

#include <optional>

namespace msgpipe::parsers {

/**
 * @class MessageParser
 * @brief Parses raw binary data into structured Message objects.
 *
 * Validates input length and performs trivial reinterpret cast.
 */
class MessageParser {
public:
    /**
     * @brief Attempts to parse input bytes into Message struct.
     * @param data Pointer to byte array
     * @param length Size of the buffer
     * @return Parsed message or nullopt if invalid
     */
    std::optional<msgpipe::protocol::Message> parse(const std::byte* data, std::size_t length) const;
};

} // namespace msgpipe::parsers