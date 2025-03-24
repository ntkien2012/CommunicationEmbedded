#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize Wi-Fi in AP mode
void wifi_init_ap(const uint8_t* ssid, const uint8_t* password);

// Transmit data over UDP
int wifi_transmit(const uint8_t* data, uint16_t len);

// Receive data over UDP
int wifi_receive(uint8_t* buffer, uint16_t max_len, uint8_t* array_out, uint16_t* array_len);

#ifdef __cplusplus
}
#endif

#endif // WIFI_H