#include <stdio.h>
#include "Bit_Message.h"

void bitMess(const uint8_t* input, uint8_t* output, int num_bytes) {
    // Check if num_bytes is even
    if (num_bytes % 2 != 0) {
        // If not even, do not process and return
        for (int i = 0; i < num_bytes; ++i) {
            output[i] = input[i];
        }
        return;
    }

    uint8_t* inverted = (uint8_t*)malloc(num_bytes * sizeof(uint8_t)); // Dynamic memory for temporary arrays
    if (inverted == NULL) {
        // Handle memory allocation errors if necessary
        for (int i = 0; i < num_bytes; ++i) {
            output[i] = input[i];
        }
        return;
    }

    // Step 1: Invert the bits of the input
    for (int i = 0; i < num_bytes; ++i) {
        inverted[i] = ~input[i]; // Bitwise NOT to invert all bits in each byte
    }

    // Step 2: Reverse the even and odd bit positions within each pair
    int total_bits = num_bytes * 8; // Total bits
    for (int i = 0; i < total_bits; i += 2) {
        int byte_idx = i / 8;           // Byte position
        int bit_idx = i % 8;            // Bit position in byte

        // Initialize bytes in output (if not initialized)
        if (i == byte_idx * 8) {
            output[byte_idx] = 0; // Reset bytes to avoid garbage values
        }

        //Extract bits from inverted array
        int bit_even = (inverted[byte_idx] >> (7 - bit_idx)) & 1;  // Even bit (i)
        int bit_odd = (inverted[byte_idx] >> (6 - bit_idx)) & 1;   // Odd bit (i + 1)

        // Swap and put into output
        if (bit_odd) {
            output[byte_idx] |= (1 << (7 - bit_idx));    // Set even bit to odd bit
        } else {
            output[byte_idx] &= ~(1 << (7 - bit_idx));   //Clear even bit
        }
        if (bit_even) {
            output[byte_idx] |= (1 << (6 - bit_idx));    // Set odd bit to even bit
        } else {
            output[byte_idx] &= ~(1 << (6 - bit_idx));   // Clear odd bit
        }
    }

    free(inverted); // Free dynamic memory
}
