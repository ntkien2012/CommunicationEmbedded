#include "Wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include <string.h>

#define WIFI_AP_IP         "192.168.4.1" // Default AP IP address
#define WIFI_AP_GATEWAY    "192.168.4.1"
#define WIFI_AP_NETMASK    "255.255.255.0"
#define UDP_PORT           12345         // UDP port for communication
#define MAX_STA_CONN       4             // Maximum number of stations that can connect
#define MAC2STR(mac)  (mac)[0], (mac)[1], (mac)[2], (mac)[3], (mac)[4], (mac)[5]

static const char* TAG = "WIFI";
static int wifi_socket = -1;

// Event handler for Wi-Fi events
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_AP_START:
                ESP_LOGI(TAG, "Wi-Fi AP started");
                break;
            case WIFI_EVENT_AP_STACONNECTED:
                wifi_event_ap_staconnected_t* connected_event = (wifi_event_ap_staconnected_t*)event_data;
                ESP_LOGI(TAG, "Station %02x:%02x:%02x:%02x:%02x:%02x connected, AID=%d",
                    connected_event->mac[0], connected_event->mac[1], connected_event->mac[2],
                    connected_event->mac[3], connected_event->mac[4], connected_event->mac[5], connected_event->aid);
                break;
            case WIFI_EVENT_AP_STADISCONNECTED:
                wifi_event_ap_stadisconnected_t* disconnected_event = (wifi_event_ap_stadisconnected_t*)event_data;
                ESP_LOGI(TAG, "Station %02x:%02x:%02x:%02x:%02x:%02x disconnected, AID=%d",
                    disconnected_event->mac[0], disconnected_event->mac[1], disconnected_event->mac[2],
                    disconnected_event->mac[3], disconnected_event->mac[4], disconnected_event->mac[5], disconnected_event->aid);
                break;
            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_AP_STAIPASSIGNED) {
        ip_event_ap_staipassigned_t* event = (ip_event_ap_staipassigned_t*)event_data;
        uint32_t ip = event->ip.addr;
        ESP_LOGI(TAG, "Assigned IP to station: %u.%u.%u.%u",
            (unsigned int)(ip & 0xFF),
            (unsigned int)((ip >> 8) & 0xFF),
            (unsigned int)((ip >> 16) & 0xFF),
            (unsigned int)((ip >> 24) & 0xFF)
        );
    }
}

// Initialize Wi-Fi in AP mode
void wifi_init_ap(const uint8_t* ssid, const uint8_t* password) {
    // Set log level to INFO for this module
    esp_log_level_set(TAG, ESP_LOG_INFO);

    // Test logging
    ESP_LOGI(TAG, "Initializing Wi-Fi AP...");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create AP network interface
    esp_netif_t* ap_netif = esp_netif_create_default_wifi_ap();
    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 4, 1);      // AP IP
    IP4_ADDR(&ip_info.gw, 192, 168, 4, 1);      // Gateway
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0); // Netmask

    // Stop the DHCP server before setting IP info
    esp_netif_dhcps_stop(ap_netif);

    ESP_ERROR_CHECK(esp_netif_set_ip_info(ap_netif, &ip_info));

    // Optionally restart the DHCP server after configuration
    esp_netif_dhcps_start(ap_netif);

    // Initialize Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &wifi_event_handler, NULL, NULL));

    // Configure Wi-Fi AP
    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.ap.ssid, (const char*)ssid, sizeof(wifi_config.ap.ssid) - 1);
    strncpy((char*)wifi_config.ap.password, (const char*)password, sizeof(wifi_config.ap.password) - 1);
    wifi_config.ap.ssid[sizeof(wifi_config.ap.ssid) - 1] = '\0';
    wifi_config.ap.password[sizeof(wifi_config.ap.password) - 1] = '\0';
    wifi_config.ap.ssid_len = strlen((char*)wifi_config.ap.ssid);
    wifi_config.ap.max_connection = MAX_STA_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    if (strlen((char*)password) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN; // Open AP if no password
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi AP started with SSID:%s, IP:" WIFI_AP_IP, ssid);

    // Initialize UDP socket
    wifi_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (wifi_socket < 0) {
        ESP_LOGE(TAG, "Failed to create UDP socket");
        return;
    }

    // Bind socket for receiving
    struct sockaddr_in local_addr = {0};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(UDP_PORT);
    local_addr.sin_addr.s_addr = inet_addr(WIFI_AP_IP);
    if (bind(wifi_socket, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        ESP_LOGE(TAG, "Failed to bind UDP socket");
        close(wifi_socket);
        wifi_socket = -1;
        return;
    }
}

// Transmit data over UDP (broadcast to all connected stations)
int wifi_transmit(const uint8_t* data, uint16_t len) {
    if (wifi_socket < 0) {
        ESP_LOGE(TAG, "UDP socket not initialized");
        return -1;
    }

    struct sockaddr_in broadcast_addr = {0};
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(UDP_PORT);
    broadcast_addr.sin_addr.s_addr = inet_addr("192.168.4.255"); // Broadcast to 192.168.4.x subnet

    int sent = sendto(wifi_socket, data, len, 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
    if (sent < 0) {
        ESP_LOGE(TAG, "Failed to send UDP data: %d", errno);
        return -1;
    }

    // ESP_LOGI(TAG, "Sent %d bytes over UDP", sent);
    return sent;
}

// Function to receive UDP data and decode it into 7-bit integer array
int wifi_receive(uint8_t* buffer, uint16_t max_len, uint8_t* array_out, uint16_t* array_len) {
    if (wifi_socket < 0) {
        ESP_LOGE(TAG, "UDP socket not initialized");
        return -1;
    }

    struct sockaddr_in source_addr;
    socklen_t socklen = sizeof(source_addr);
    int received = recvfrom(wifi_socket, buffer, max_len, 0, (struct sockaddr*)&source_addr, &socklen);
    if (received < 0) {
        ESP_LOGE(TAG, "Failed to receive UDP data: %d", errno);
        return -1;
    }

    uint32_t ip = source_addr.sin_addr.s_addr;
    ESP_LOGI(TAG, "Received %d bytes over UDP from %u.%u.%u.%u:%u",
        received,
        (unsigned int)(ip & 0xFF),
        (unsigned int)((ip >> 8) & 0xFF),
        (unsigned int)((ip >> 16) & 0xFF),
        (unsigned int)((ip >> 24) & 0xFF),
        (unsigned int)ntohs(source_addr.sin_port));

    // Decode 7-bit data into array
    if (received < 2) { // At least 2 bytes are required for the length
        ESP_LOGE(TAG, "Received data too short");
        return -1;
    }

    // Get the number of elements from the first 2 bytes
    uint16_t num_elements = *(uint16_t*)buffer;
    if (num_elements > *array_len) { // Check if the output array has enough space
        ESP_LOGE(TAG, "Array output too small: %d > %d", num_elements, *array_len);
        return -1;
    }

    // Decrypt data
    uint8_t* data = buffer + 2; // Skip first 2 bytes
    int byte_len = received - 2; // Actual data length
    uint32_t temp = 0;          // Temporary variable to accumulate bits
    int bits = 0;               // Number of bits accumulated
    int element_count = 0;      // Count the number of decoded elements

    for (int i = 0; i < byte_len && element_count < num_elements; i++) {
        temp |= (uint32_t)data[i] << bits; // Add bytes to temp
        bits += 8;
        while (bits >= 7 && element_count < num_elements) {
            array_out[element_count] = temp & 0x7F; // Take the lowest 7 bits
            temp >>= 7;                            // Right shift 7 bits
            bits -= 7;
            element_count++;
        }
    }

    // Check if there is not enough data to decode all the elements
    if (element_count < num_elements) {
        ESP_LOGE(TAG, "Not enough data to decode %d elements (got %d)", num_elements, element_count);
        return -1;
    }

    // Update output array length
    *array_len = num_elements;
    return num_elements; // Returns the number of elements in the array
}