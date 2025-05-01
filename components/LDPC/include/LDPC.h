#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <check_matrix.h>

typedef struct {
    const uint16_t *row_ptr;  // size M+1
    const uint16_t *col_idx;  // size NNZ
    uint16_t m, n;
    uint16_t max_iter;
    uint8_t *threshold;       // size n
} BitFlipDecoder;


void decoder_init(BitFlipDecoder *dec);
int decode(BitFlipDecoder *dec,
    const uint8_t *received,
    uint8_t *info_bits);