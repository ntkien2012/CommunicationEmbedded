#ifndef BITMESS_H
#define BITMESS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function to manipulate 128 bits (16 bytes)
void bitMess(const uint8_t* input, uint8_t* output, int num_bytes);

#ifdef __cplusplus
}
#endif

#endif