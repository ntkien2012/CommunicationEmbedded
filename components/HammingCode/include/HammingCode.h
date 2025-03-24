#ifndef HAMMING74_H
#define HAMMING74_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void hamming74_channel_coding(const uint8_t* in, uint8_t* out, size_t input_len);

void hamming74_channel_decoding(const uint8_t* in, uint8_t* out, size_t in_len);

#ifdef __cplusplus
}
#endif

#endif