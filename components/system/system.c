#include <stdio.h>
#include "system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include "esp_log.h"
#include "esp_private/esp_clk.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"

#define LOG_INTERVAL_MS 2000  // Log every 2 seconds
#define STATS_BUFFER_SIZE 2048
static const char *TAG = "transmit_task";
static const char *TAG_1 = "receive_task";

extern int wifi_receive(uint8_t* buffer, uint16_t max_len, uint8_t* array_out, uint16_t* array_len);

void system_init(System_Context* ctx, const uint8_t* ssid, const uint8_t* password, const uint8_t* key, uint8_t sysid, uint8_t compid) {
    strncpy((char*)ctx->wifi_ssid, (const char*)ssid, sizeof(ctx->wifi_ssid) - 1);
    ctx->wifi_ssid[sizeof(ctx->wifi_ssid) - 1] = '\0';
    strncpy((char*)ctx->wifi_password, (const char*)password, sizeof(ctx->wifi_password) - 1);
    ctx->wifi_password[sizeof(ctx->wifi_password) - 1] = '\0';

    wifi_init_ap(ctx->wifi_ssid, ctx->wifi_password);

    memcpy(ctx->aes_key, key, 32);
    AES256_init(&ctx->aes_ctx, ctx->aes_key);

    ctx->sysid = sysid;
    ctx->compid = compid;
    ctx->seq = 0;
}

// Function to handle padding for blocks that are smaller than 16 bytes
void pad_block(uint8_t *block, size_t block_size, size_t padded_size) {
    for (size_t i = block_size; i < padded_size; i++) {
        block[i] = padded_size - block_size;  // PKCS#5/PKCS#7 padding
    }
}

static void transmit_task(void* arg) {
    System_Context* ctx = (System_Context*)arg;
    const char* plaintext = "MAVLINK HEARTBEAT";
    uint16_t plaintext_len = strlen(plaintext);
    size_t num_blocks = (plaintext_len + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;
    uint8_t final_message[AES_BLOCK_SIZE*num_blocks];  // Buffer to hold the concatenated message
    size_t final_message_len = AES_BLOCK_SIZE*num_blocks;  // Length of concatenated final message
    size_t concatenated_index;
    
    const int WDT_TIMEOUT_MS = 10000;
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = WDT_TIMEOUT_MS,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = false
    };
    esp_task_wdt_init(&wdt_config);
    
    esp_err_t err = esp_task_wdt_add(NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add task to WDT");
    }
    
    esp_task_wdt_reset();

    while (1) {
        esp_task_wdt_reset();
        // Reset concatenated_index for each new loop
        concatenated_index = 0;
        memset(final_message, 0, final_message_len);

        esp_task_wdt_reset();
        
        for (size_t block_index = 0; block_index < num_blocks; block_index++) {
            if (block_index % 2 == 0) {
                esp_task_wdt_reset();
            }
            
            uint8_t padded_plaintext[AES_BLOCK_SIZE] = {0};
            size_t start_index = block_index * AES_BLOCK_SIZE;
            size_t block_size = (plaintext_len - start_index < AES_BLOCK_SIZE) ? plaintext_len - start_index : AES_BLOCK_SIZE;
    
            // Step 1: Pad the block if it's smaller than 16 bytes
            memcpy(padded_plaintext, &plaintext[start_index], block_size);
            if (block_size < AES_BLOCK_SIZE) {
                pad_block(padded_plaintext, block_size, AES_BLOCK_SIZE);
            }
    
            // Step 2: AES-256 encryption
            uint8_t ciphertext[AES_BLOCK_SIZE] = {0};
            AES256_encrypt(&ctx->aes_ctx, padded_plaintext, ciphertext);  // Use a real AES context
    
            // Step 3: bitMess transformation
            uint8_t bitmessed[AES_BLOCK_SIZE] = {0};
            bitMess(ciphertext, bitmessed, sizeof(ciphertext));
            
            memcpy(&final_message[concatenated_index], bitmessed, AES_BLOCK_SIZE);
            concatenated_index += AES_BLOCK_SIZE;
        }

        esp_task_wdt_reset();

        // Step 4: MAVLink encoding
        mavlink_message_t msg;
        msg.header = MAVLINK_STX;
        msg.len = final_message_len;
        msg.seq = ctx->seq++;
        msg.sysid = ctx->sysid;
        msg.compid = ctx->compid;
        msg.msgid = 0;
        memcpy(msg.payload, final_message, final_message_len);
        
        // Check buffer size before use
        if (final_message_len + 8 > sizeof(uint8_t) * (final_message_len + 8)) {
            ESP_LOGE(TAG, "mavlink_buffer would overflow: %u > %u", final_message_len + 8, sizeof(uint8_t) * (final_message_len + 8));
            esp_task_wdt_reset();
            vTaskDelay(500 / portTICK_PERIOD_MS);
            continue;
        }
        
        uint8_t mavlink_buffer[final_message_len+8];
        mavlink_encode(&msg, mavlink_buffer);

        esp_task_wdt_reset();

        // Step 5: Hamming (7,4) encoding
        // Check buffer size before use
        if ((final_message_len + 8) / 4 * 7 > sizeof(uint8_t) * ((final_message_len + 8) / 4 * 7)) {
            ESP_LOGE(TAG, "hamming_encoded would overflow: %u > %u", (final_message_len + 8) / 4 * 7, sizeof(uint8_t) * ((final_message_len + 8) / 4 * 7));
            esp_task_wdt_reset();
            vTaskDelay(500 / portTICK_PERIOD_MS);
            continue;
        }
        
        uint8_t hamming_encoded[(final_message_len+8)/4*7];
        memset(hamming_encoded, 0, sizeof(hamming_encoded));
        hamming74_channel_coding(mavlink_buffer, hamming_encoded, final_message_len+8);

        esp_task_wdt_reset();

        // Step 6: Transmit
        int sent = wifi_transmit(hamming_encoded, sizeof(hamming_encoded));
        if (sent > 0) {
            esp_task_wdt_reset();
        } else {
            printf("Transmit failed\n");
            esp_task_wdt_reset();
        }

        // Check stack usage (uncomment if debugging needed)
        // UBaseType_t stack_high_water_mark = uxTaskGetStackHighWaterMark(NULL);
        // printf("Stack High Water Mark: %u bytes remaining\n", stack_high_water_mark * sizeof(StackType_t));
        
        esp_task_wdt_reset();
        
        memset(mavlink_buffer, 0, sizeof(mavlink_buffer));
        memset(hamming_encoded, 0, sizeof(hamming_encoded));

        vTaskDelay(50 / portTICK_PERIOD_MS);
        esp_task_wdt_reset();
        vTaskDelay(1950 / portTICK_PERIOD_MS);
    }
}

static void receive_task(void* arg) {
    System_Context* ctx = (System_Context*)arg;
    // Define the built-in LED pin (commonly GPIO 2 on many ESP32 boards)
    const gpio_num_t LED_PIN = GPIO_NUM_2;
    // Configure LED pin
    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);  // Start with LED off

    while (1) {
        // Dynamic buffer to receive data from UDP
        uint8_t* rx_buffer = malloc(1024);
        uint8_t* array_out = malloc(1024);
        uint16_t array_len = 1024;
        if (!rx_buffer) {
            ESP_LOGE(TAG_1, "Failed to allocate rx_buffer");
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        // Get data of any length
        int rx_len = wifi_receive(rx_buffer, 1024, array_out, &array_len);
        // ESP_LOGI(TAG_1, "Received %d bytes", rx_len); // Debug
        if (rx_len <= 0) {
            ESP_LOGW(TAG_1, "Receive failed or no data (%d bytes)", rx_len);
            free(rx_buffer);
            free(array_out);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        // printf("rx buffer Data: ");
        // for (int i = 0; i < rx_len; i++) printf("%d ", array_out[i]);
        // printf("\n");

        // Check valid length for Hamming (must be multiple of 7)
        // if (rx_len % 7 != 0) {
        //     ESP_LOGW(TAG_1, "Invalid Hamming length: %d bytes (must be multiple of 7)", rx_len);
        //     free(rx_buffer);
        //     vTaskDelay(100 / portTICK_PERIOD_MS);
        //     continue;
        // }

        // Hamming decoding (7,4) → 4 bytes from every 7 bytes
        size_t hamming_decoded_len = rx_len / 2;
        uint8_t* hamming_decoded = malloc(hamming_decoded_len);
        hamming74_channel_decoding(array_out, hamming_decoded, rx_len);
        if (!hamming_decoded) {
            ESP_LOGE(TAG_1, "Failed to allocate hamming_decoded");
            free(rx_buffer);
            free(array_out);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }
        // printf("Hamming Decoded Data: ");
        // for (int i = 0; i < hamming_decoded_len; i++) printf("%02X ", hamming_decoded[i]);
        // printf("\n");
        free(rx_buffer); // Free the receive buffer
        free(array_out);

        // MAVLink Decoding
        mavlink_message_t rxmsg;
        int decode_result = mavlink_decode(&rxmsg, hamming_decoded, hamming_decoded_len);
        if (decode_result < 0) {
            ESP_LOGW(TAG_1, "MAVLink decode failed");
            free(hamming_decoded);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        // Check payload length (must be multiple of 16 due to AES)
        if (rxmsg.len % AES_BLOCK_SIZE != 0) {
            ESP_LOGW(TAG_1, "Invalid MAVLink payload length: %d (must be multiple of %d)", rxmsg.len, AES_BLOCK_SIZE);
            free(hamming_decoded);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        // AES decryption with multiple blocks
        size_t num_blocks = rxmsg.len / AES_BLOCK_SIZE;
        uint8_t* bitmess_reversed = malloc(rxmsg.len);
        uint8_t* decrypted = malloc(rxmsg.len);
        if (!bitmess_reversed || !decrypted) {
            ESP_LOGE(TAG_1, "Failed to allocate buffers for AES");
            free(hamming_decoded);
            free(bitmess_reversed);
            free(decrypted);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        bitMess(rxmsg.payload, bitmess_reversed, rxmsg.len);
        // printf("Ciphertext: ");
        // for (int i = 0; i < rxmsg.len; i++) printf("%02X ", bitmess_reversed[i]);
        // printf("\n");
        
        for (size_t i = 0; i < num_blocks; i++) {
            AES256_decrypt(&ctx->aes_ctx, &bitmess_reversed[i * AES_BLOCK_SIZE], &decrypted[i * AES_BLOCK_SIZE]);
        }

        // printf("Decrypted: ");
        // for (int i = 0; i < rxmsg.len; i++) printf("%02X ", decrypted[i]);
        // printf("\n");

        // Process padding to get plaintext
        uint8_t padding_value = decrypted[rxmsg.len - 1];
        size_t plaintext_len = (padding_value > 0 && padding_value <= AES_BLOCK_SIZE) ? rxmsg.len - padding_value : rxmsg.len;
        char* received_plaintext = malloc(plaintext_len + 1); // +1 cho null terminator
        if (!received_plaintext) {
            ESP_LOGE(TAG_1, "Failed to allocate received_plaintext");
            free(hamming_decoded);
            free(bitmess_reversed);
            free(decrypted);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        memcpy(received_plaintext, decrypted, plaintext_len);
        received_plaintext[plaintext_len] = '\0';

        ESP_LOGI(TAG_1, "Received and decoded: '%s' (%d bytes)", received_plaintext, plaintext_len);

        // Blink LED 3 times when decoding succeeds
        for (int i = 0; i < 3; i++) {
            gpio_set_level(LED_PIN, 1);  // LED ON
            vTaskDelay(200 / portTICK_PERIOD_MS);  // Wait 200ms
            gpio_set_level(LED_PIN, 0);  // LED OFF
            vTaskDelay(200 / portTICK_PERIOD_MS);  // Wait 200ms
        }

        // Check stack
        // UBaseType_t stack_high_water_mark = uxTaskGetStackHighWaterMark(NULL);
        // ESP_LOGI(TAG_1, "Stack remaining: %u bytes", stack_high_water_mark * sizeof(StackType_t));

        // Free up memory
        free(hamming_decoded);
        free(bitmess_reversed);
        free(decrypted);
        free(received_plaintext);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// Log task list using vTaskList
void log_tasks_info(void) {
    char task_list[STATS_BUFFER_SIZE];
    vTaskList(task_list);
    printf("=== Task List ===\n%s\n", task_list);
}

// Log runtime stats using vTaskGetRunTimeStats
void log_runtime_stats(void) {
    char runtime_stats[STATS_BUFFER_SIZE];
    vTaskGetRunTimeStats(runtime_stats);
    printf("=== Task Runtime Stats ===\n%s\n", runtime_stats);
}

// Monitoring task to log both task list and runtime stats
void monitoring_task(void *param) {
    while (1) {
        log_tasks_info();
        log_runtime_stats();
        vTaskDelay(pdMS_TO_TICKS(LOG_INTERVAL_MS));
    }
}


void system_launch(System_Context* ctx) {
    xTaskCreatePinnedToCore(transmit_task, "Transmit_task", 2048*3, ctx, 1, NULL, 1);
    xTaskCreate(receive_task, "receive_task", 2048*3, ctx, 6, NULL);
    // xTaskCreate(monitoring_task, "MonitoringTask", 4096, NULL, 1, NULL);
}