#include "packet.hpp"
#include <arpa/inet.h>
#include <cstring>

namespace {
// writes the 8-byte header shared by DATA and ACK; reserved bytes are zeroed
void writeHeader(uint8_t* buf, uint8_t type, uint8_t sender_id, uint32_t seq)
{
    std::memset(buf, 0, packet::HEADER_SIZE);
    buf[packet::TYPE_OFFSET] = type;
    buf[packet::SENDER_OFFSET] = sender_id;
    uint32_t seq_net = htonl(seq);
    std::memcpy(buf + packet::SEQ_OFFSET, &seq_net, packet::SEQ_SIZE);
}
}

size_t packet::encodeData(uint8_t* buf, uint8_t sender_id, uint32_t first_seq, const uint32_t * payloads, size_t n)
{
    if (n < 1 || n > packet::MAX_MSGS_PER_PACKET) {
        return 0;
    }

    writeHeader(buf, packet::TYPE_DATA, sender_id, first_seq);
    for (size_t i = 0; i < n; i++) {
        uint32_t payload_net = htonl(payloads[i]);
        std::memcpy(buf + packet::PAYLOAD_OFFSET + packet::MSG_SIZE * i, &payload_net, packet::MSG_SIZE);
    }
    return packet::HEADER_SIZE + packet::MSG_SIZE * n;
}

size_t packet::encodeAck(uint8_t* buf, uint8_t sender_id, uint32_t cumulative_seq)
{
    writeHeader(buf, packet::TYPE_ACK, sender_id, cumulative_seq);
    return packet::HEADER_SIZE;
}

bool packet::decode(const uint8_t* buf, size_t len, Decoded& out)
{
    if (len < packet::HEADER_SIZE) {
        return false;
    }

    out.type = buf[packet::TYPE_OFFSET];
    out.sender_id = buf[packet::SENDER_OFFSET];
    uint32_t seq_net;
    std::memcpy(&seq_net, buf + packet::SEQ_OFFSET, packet::SEQ_SIZE);
    out.seq = ntohl(seq_net);

    if (out.type == packet::TYPE_ACK) {
        out.n = 0;
        return len == packet::HEADER_SIZE;
    }
    if (out.type != packet::TYPE_DATA) {
        return false;
    }

    // data: whole payloads only, 1 to MAX_MSGS_PER_PACKET of them
    if (len > packet::MAX_PACKET_SIZE || (len - packet::HEADER_SIZE) % packet::MSG_SIZE != 0) {
        return false;
    }
    out.n = (len - packet::HEADER_SIZE) / packet::MSG_SIZE;
    if (out.n == 0) {
        return false;
    }
    for (size_t i = 0; i < out.n; i++) {
        uint32_t payload_net;
        std::memcpy(&payload_net, buf + packet::PAYLOAD_OFFSET + packet::MSG_SIZE * i, packet::MSG_SIZE);
        out.payloads[i] = ntohl(payload_net);
    }
    return true;
}
