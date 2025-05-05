#pragma once

#include <cstddef>
#include <cstdint>

namespace msgpipe::protocol {

/**
 * @brief Network message structure used for communication over UDP/TCP.
 *
 * The structure is packed to match the exact byte layout expected on the wire.
 * Do not reorder or realign fields without adjusting corresponding parsers and serialization.
 */
#pragma pack(push, 1) // Ensure no padding between struct fields
struct Message {
    uint16_t size;     ///< Total size of the message (expected to be sizeof(Message))
    uint8_t  type;     ///< Application-defined message type
    uint64_t id;       ///< Unique identifier for deduplication
    uint64_t data;     ///< Main payload data
};
#pragma pack(pop)

/**
 * @brief Expected serialized size of the Message structure.
 */
inline constexpr std::size_t ExpectedMessageSize = sizeof(Message);

} // namespace msgpipe::protocol
