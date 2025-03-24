#include <stdio.h>
#include "MAVLink.h"

// CRC accumulation function (X.25 standard)
void crc_accumulate(uint8_t data, uint16_t* crcAccum) {
    uint8_t tmp = data ^ (*crcAccum & 0xFF);
    tmp ^= (tmp << 4);
    *crcAccum = (*crcAccum >> 8) ^ ((uint16_t)tmp << 8) ^ ((uint16_t)tmp << 3) ^ ((uint16_t)tmp >> 4);
}

// Initialize CRC
void crc_init(uint16_t* crcAccum) {
    *crcAccum = X25_INIT_CRC;
}

// Calculate CRC over a buffer
uint16_t crc_calculate(const uint8_t* pBuffer, uint16_t length) {
    uint16_t crcTmp;
    crc_init(&crcTmp);
    while (length--) {
        crc_accumulate(*pBuffer++, &crcTmp);
    }
    return crcTmp;
}

// Accumulate CRC over a buffer
void crc_accumulate_buffer(uint16_t* crcAccum, const uint8_t* pBuffer, uint16_t length) {
    while (length--) {
        crc_accumulate(*pBuffer++, crcAccum);
    }
}

// Start checksum calculation for a message
void mavlink_start_checksum(mavlink_message_t* msg) {
    crc_init(&msg->checksum);
}

// Update checksum with a single byte
void mavlink_update_checksum(mavlink_message_t* msg, uint8_t c) {
    crc_accumulate(c, &msg->checksum);
}

// Encode a MAVLink message into a buffer
int mavlink_encode(const mavlink_message_t* msg, uint8_t* buffer) {
    int offset = 0;

    // Header
    buffer[offset++] = MAVLINK_STX;      // Start byte
    buffer[offset++] = msg->len;         // Payload length
    buffer[offset++] = msg->seq;         // Sequence number
    buffer[offset++] = msg->sysid;       // System ID
    buffer[offset++] = msg->compid;      // Component ID
    buffer[offset++] = msg->msgid;       // Message ID
    // buffer[offset++] = msg->segment_index; // Segment index
    // buffer[offset++] = msg->total_segments; // Total segments

    // Initialize checksum (after STX)
    mavlink_start_checksum((mavlink_message_t*)msg);
    mavlink_update_checksum((mavlink_message_t*)msg, msg->len);
    mavlink_update_checksum((mavlink_message_t*)msg, msg->seq);
    mavlink_update_checksum((mavlink_message_t*)msg, msg->sysid);
    mavlink_update_checksum((mavlink_message_t*)msg, msg->compid);
    mavlink_update_checksum((mavlink_message_t*)msg, msg->msgid);
    // mavlink_update_checksum((mavlink_message_t*)msg, msg->segment_index);
    // mavlink_update_checksum((mavlink_message_t*)msg, msg->total_segments);

    // Payload
    for (int i = 0; i < msg->len; ++i) {
        buffer[offset++] = msg->payload[i];
        mavlink_update_checksum((mavlink_message_t*)msg, msg->payload[i]);
    }

    // Add CRC extra (placeholder; define your own table or remove if not needed)
#ifdef MAVLINK_CRC_EXTRA
    // Example: Replace with actual message-specific CRC extra from mavlink library
    uint8_t crc_extra = 0; // Placeholder (e.g., MAVLINK_MESSAGE_CRC_(msg->msgid))
    mavlink_update_checksum((mavlink_message_t*)msg, crc_extra);
#endif

    // Append checksum
    buffer[offset++] = msg->checksum & 0xFF;        // Low byte
    buffer[offset++] = (msg->checksum >> 8) & 0xFF; // High byte

    return offset; // Total length of encoded message
}

// Decode a MAVLink message from a buffer
int mavlink_decode(mavlink_message_t* rxmsg, const uint8_t* buffer, uint16_t length) {
    mavlink_state_t state = MAVLINK_STATE_IDLE;
    int pos = 0;

    if (length < 8) { // Minimum: STX + len + seq + sysid + compid + msgid + CRC (2 bytes)
        return -1; // Buffer too short
    }

    while (pos < length && state != MAVLINK_STATE_GOT_CRC1) {
        switch (state) {
            case MAVLINK_STATE_IDLE:
                if (buffer[pos] == MAVLINK_STX) {
                    rxmsg->header = buffer[pos];
                    mavlink_start_checksum(rxmsg);
                    state = MAVLINK_STATE_GOT_STX;
                    pos++;
                } else {
                    return -1; // Invalid start byte
                }
                break;

            case MAVLINK_STATE_GOT_STX:
                rxmsg->len = buffer[pos];
                mavlink_update_checksum(rxmsg, buffer[pos]);
                state = MAVLINK_STATE_GOT_LENGTH;
                pos++;
                break;

            case MAVLINK_STATE_GOT_LENGTH:
                rxmsg->seq = buffer[pos];
                mavlink_update_checksum(rxmsg, buffer[pos]);
                state = MAVLINK_STATE_GOT_SEQ;
                pos++;
                break;

            case MAVLINK_STATE_GOT_SEQ:
                rxmsg->sysid = buffer[pos];
                mavlink_update_checksum(rxmsg, buffer[pos]);
                state = MAVLINK_STATE_GOT_SYSID;
                pos++;
                break;

            case MAVLINK_STATE_GOT_SYSID:
                rxmsg->compid = buffer[pos];
                mavlink_update_checksum(rxmsg, buffer[pos]);
                state = MAVLINK_STATE_GOT_COMPID;
                pos++;
                break;

            case MAVLINK_STATE_GOT_COMPID:
                rxmsg->msgid = buffer[pos];
                mavlink_update_checksum(rxmsg, buffer[pos]);
                state = MAVLINK_STATE_GOT_MSGID;
                pos++;
                break;

            case MAVLINK_STATE_GOT_MSGID:
                if (pos + rxmsg->len + 2 > length) {
                    return -1; // Buffer too short for payload + CRC
                }
                for (int i = 0; i < rxmsg->len; ++i) {
                    rxmsg->payload[i] = buffer[pos + i];
                    mavlink_update_checksum(rxmsg, buffer[pos + i]);
                }
#ifdef MAVLINK_CRC_EXTRA
                // Placeholder: Replace with actual message-specific CRC extra
                uint8_t crc_extra = 0; // e.g., MAVLINK_MESSAGE_CRC_(rxmsg->msgid)
                mavlink_update_checksum(rxmsg, crc_extra);
#endif
                pos += rxmsg->len;
                state = MAVLINK_STATE_GOT_PAYLOAD;
                break;

            // case MAVLINK_STATE_GOT_SEGMENT_INDEX:
            //     rxmsg->msgid = buffer[pos];
            //     mavlink_update_checksum(rxmsg, buffer[pos]);
            //     state = MAVLINK_STATE_GOT_MSGID;
            //     pos++;
            //     break;

            // case MAVLINK_STATE_GOT_TOTAL_SEGMENTS:
            //     rxmsg->msgid = buffer[pos];
            //     mavlink_update_checksum(rxmsg, buffer[pos]);
            //     state = MAVLINK_STATE_GOT_MSGID;
            //     pos++;
            //     break;

            case MAVLINK_STATE_GOT_PAYLOAD:
                rxmsg->checksum_calculator[0] = buffer[pos];
                rxmsg->checksum_calculator[1] = buffer[pos + 1];
                state = MAVLINK_STATE_GOT_CRC1;
                pos += 2;
                break;

            default:
                return -1; // Invalid state
        }
    }

    if (state != MAVLINK_STATE_GOT_CRC1) {
        return -1; // Incomplete message
    }

    // Verify checksum
    uint8_t crc_temp[2];
    crc_temp[0] = rxmsg->checksum & 0xFF;       // Low byte
    crc_temp[1] = (rxmsg->checksum >> 8) & 0xFF; // High byte
    if (crc_temp[0] == rxmsg->checksum_calculator[0] && crc_temp[1] == rxmsg->checksum_calculator[1]) {
        return pos; // Success, return number of bytes processed
    } else {
        return -1; // Checksum mismatch
    }
}
