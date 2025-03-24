#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "system.h"

void app_main(void) {
    // System configuration
    uint8_t ssid[] = "ESP32 WIFI COMMUNICATION";
    uint8_t password[] = "12345678";
    uint8_t key[32] = { /* 32-byte AES-256 key */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
    };
    uint8_t sysid = 1;    // MAVLink system ID
    uint8_t compid = 200; // MAVLink component ID   

    // Initialize system
    System_Context ctx;
    system_init(&ctx, ssid, password, key, sysid, compid);

    // Launch transmit and receive tasks
    system_launch(&ctx);

    // Keep main task alive
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait 1 second
    }
}