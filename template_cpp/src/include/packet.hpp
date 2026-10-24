#pragma once

#include <cstddef>
#include <cstdint>

namespace packet {
constexpr size_t   HEADER_SIZE = 8; // same as PAYLOAD_OFFSET

constexpr uint8_t  TYPE_DATA   = 0;
constexpr uint8_t  TYPE_ACK   = 1;
constexpr size_t   TYPE_OFFSET   = 0;

constexpr size_t   SENDER_OFFSET   = 1;
constexpr size_t   SENDER_SIZE = 1;

// There two bytes reserved between sender and seq

constexpr size_t   SEQ_OFFSET  = 4;
constexpr size_t   SEQ_SIZE = 4;

constexpr size_t   PAYLOAD_OFFSET = HEADER_SIZE;
constexpr size_t   MSG_SIZE = 4;
constexpr size_t   MAX_MSGS_PER_PACKET = 8;
constexpr size_t MAX_PACKET_SIZE = HEADER_SIZE + MAX_MSGS_PER_PACKET * MSG_SIZE;
}