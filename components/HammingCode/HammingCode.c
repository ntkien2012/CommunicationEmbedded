#include "HammingCode.h"
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "esp_log.h"

static const char *TAG = "HammingCode";

// Helper: Encode 4 bits into 7-bit Hamming (7,4) code
static void hamming74_encode(uint8_t nibble, uint8_t* encoded) {
    uint8_t d1 = (nibble >> 3) & 1;
    uint8_t d2 = (nibble >> 2) & 1;
    uint8_t d3 = (nibble >> 1) & 1;
    uint8_t d4 = nibble & 1;

    uint8_t p1 = d1 ^ d2 ^ d4;
    uint8_t p2 = d1 ^ d3 ^ d4;
    uint8_t p3 = d2 ^ d3 ^ d4;

    *encoded = (p1 << 6) | (p2 << 5) | (d1 << 4) | (p3 << 3) | (d2 << 2) | (d3 << 1) | d4;
}

// Helper: Decode 7-bit Hamming (7,4) code into 4 bits, correcting single-bit errors
static void hamming74_decode(uint8_t codeword, uint8_t* nibble) {
    uint8_t P1 = (codeword >> 6) & 1;
    uint8_t P2 = (codeword >> 5) & 1;
    uint8_t D1 = (codeword >> 4) & 1;
    uint8_t P3 = (codeword >> 3) & 1;
    uint8_t D2 = (codeword >> 2) & 1;
    uint8_t D3 = (codeword >> 1) & 1;
    uint8_t D4 = codeword & 1;

    uint8_t S1 = P1 ^ D1 ^ D2 ^ D4;
    uint8_t S2 = P2 ^ D1 ^ D3 ^ D4;
    uint8_t S3 = P3 ^ D2 ^ D3 ^ D4;

    uint8_t syndrome = (S3 << 2) | (S2 << 1) | S1;
    if (syndrome != 0 && syndrome <= 7) {
        ESP_LOGW(TAG, "Error detected at position %d, correcting", syndrome);
        codeword ^= (1 << (6 - (syndrome - 1)));
        D1 = (codeword >> 4) & 1;
        D2 = (codeword >> 2) & 1;
        D3 = (codeword >> 1) & 1;
        D4 = codeword & 1;
    }

    nibble[0] = D1;
    nibble[1] = D2;
    nibble[2] = D3;
    nibble[3] = D4;
}

void hamming74_channel_coding(const uint8_t* in, uint8_t* out, size_t input_len) {
    // Calculate the output length based on input size
    size_t num_nibbles = input_len * 2;
    size_t output_len = (size_t)ceil(num_nibbles * 7.0 / 8.0);  // 7 bits per 4-bit input, divided by 8 for byte count
    
    memset(out, 0, output_len);

    uint8_t nibbles[num_nibbles];
    for (size_t i = 0; i < input_len; i++) {
        nibbles[i * 2] = (in[i] >> 4) & 0x0F;
        nibbles[i * 2 + 1] = in[i] & 0x0F;
    }

    for (size_t i = 0; i < num_nibbles; i++) {
        uint8_t encoded;
        hamming74_encode(nibbles[i], &encoded);

        size_t bit_pos = i * 7;
        size_t byte_pos = bit_pos / 8;
        size_t shift = bit_pos % 8;

        if (shift <= 1) {
            out[byte_pos] |= encoded << (1 - shift);
        } else {
            out[byte_pos] |= encoded >> (shift - 1);
            out[byte_pos + 1] |= encoded << (9 - shift);
        }
    }
}

// Hamming(7,4) decoding function with each element being 7 bits
void hamming74_channel_decoding(const uint8_t* in, uint8_t* out, size_t in_len) {
    size_t num_blocks = in_len;         // Number of Hamming blocks (7 bits per byte)
    size_t nibble_len = num_blocks * 4; // Total original data bits
    size_t out_len = num_blocks / 2;    // Number of output bytes (2 nibbles = 1 byte)

    if (num_blocks % 2 != 0) {
        ESP_LOGW(TAG, "Input length must be even (got %zu)", num_blocks);
        return;
    }

    uint8_t* nibbles = malloc(nibble_len);
    if (!nibbles) {
        ESP_LOGE(TAG, "Failed to allocate nibbles");
        return;
    }
    memset(out, 0, out_len);

    //Decode each 7-bit block
    for (size_t i = 0; i < num_blocks; i++) {
        uint8_t codeword = in[i] & 0x7F; // Take the low 7 bits from each byte
        hamming74_decode(codeword, &nibbles[i * 4]);
    }

    // Concatenate 2 nibbles into 1 byte
    for (size_t i = 0; i < out_len; i++) {
        uint8_t high_nibble = (nibbles[i * 8] << 3) | (nibbles[i * 8 + 1] << 2) | (nibbles[i * 8 + 2] << 1) | nibbles[i * 8 + 3];
        uint8_t low_nibble = (nibbles[i * 8 + 4] << 3) | (nibbles[i * 8 + 5] << 2) | (nibbles[i * 8 + 6] << 1) | nibbles[i * 8 + 7];
        out[i] = (high_nibble << 4) | low_nibble;
    }

    free(nibbles);
}