#ifndef MAVLINK_H
#define MAVLINK_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// MAVLink constants
#define X25_INIT_CRC 0xFFFF  // Initial CRC value for X.25
#define MAVLINK_STX 0xFE     // Start byte for MAVLink v1
#define MAVLINK_MAX_PAYLOAD_LEN 255  // Maximum payload length

// MAVLink state machine states
typedef enum {
    MAVLINK_STATE_IDLE,
    MAVLINK_STATE_GOT_STX,
    MAVLINK_STATE_GOT_LENGTH,
    MAVLINK_STATE_GOT_SEQ,
    MAVLINK_STATE_GOT_SYSID,
    MAVLINK_STATE_GOT_COMPID,
    MAVLINK_STATE_GOT_MSGID,
    // MAVLINK_STATE_GOT_SEGMENT_INDEX,
    // MAVLINK_STATE_GOT_TOTAL_SEGMENTS,
    MAVLINK_STATE_GOT_PAYLOAD,
    MAVLINK_STATE_GOT_CRC,
    MAVLINK_STATE_GOT_CRC1
} mavlink_state_t;

// MAVLink message structure
typedef struct {
    uint8_t header;              // Start byte (e.g., 0xFE)
    uint8_t len;                 // Payload length
    uint8_t seq;                 // Sequence number
    uint8_t sysid;               // System ID
    uint8_t compid;              // Component ID
    uint8_t msgid;               // Message ID
    uint8_t payload[MAVLINK_MAX_PAYLOAD_LEN];  // Payload data
    uint8_t payload_received;    // <== THÊM DÒNG NÀY
    uint16_t checksum;           // Calculated checksum
    uint8_t checksum_calculator[2]; // Received checksum bytes
} mavlink_message_t;

// CRC functions
void crc_accumulate(uint8_t data, uint16_t* crcAccum);
void crc_init(uint16_t* crcAccum);
uint16_t crc_calculate(const uint8_t* pBuffer, uint16_t length);
void crc_accumulate_buffer(uint16_t* crcAccum, const uint8_t* pBuffer, uint16_t length);

// MAVLink checksum helpers
void mavlink_start_checksum(mavlink_message_t* msg);
void mavlink_update_checksum(mavlink_message_t* msg, uint8_t c);

// MAVLink encode/decode
int mavlink_encode(const mavlink_message_t* msg, uint8_t* buffer);
int mavlink_decode(mavlink_message_t* rxmsg, const uint8_t* buffer, uint16_t length);

bool mavlink_parse_byte(mavlink_state_t *state, mavlink_message_t *msg, uint8_t byte);


#ifdef __cplusplus
}
#endif

#endif
