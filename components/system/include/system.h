#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include "AES256.h"
#include "Bit_Message.h"
#include "LDPC.h"
#include "MAVLink.h"
#include "Wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

// System context structure
typedef struct {
    AES256_Context aes_ctx;   // AES-256 context
    uint8_t wifi_ssid[32];    // Wi-Fi SSID
    uint8_t wifi_password[64]; // Wi-Fi password
    uint8_t aes_key[32];      // AES-256 key
    uint8_t sysid;            // MAVLink system ID
    uint8_t compid;           // MAVLink component ID
    uint8_t seq;              // MAVLink sequence number
} System_Context;

// Initialize the system
void system_init(System_Context* ctx, const uint8_t* ssid, const uint8_t* password, const uint8_t* key, uint8_t sysid, uint8_t compid);

// Launch the system with transmit and receive tasks
void system_launch(System_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif