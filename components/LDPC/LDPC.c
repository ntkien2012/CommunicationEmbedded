// ldpc_decoder.c
#include "LDPC.h"
#include "esp_log.h"

#define TAG "LDPC_DECODER"


void decoder_init(BitFlipDecoder *dec) {
    dec->row_ptr  = row_ptr;
    dec->col_idx  = col_idx;
    dec->m        = H_M;
    dec->n        = H_N;
    dec->max_iter = 50;
    // Calculate col_weights[j]
    uint16_t *col_weights = malloc(H_N * sizeof(uint16_t));
    memset(col_weights, 0, H_N * sizeof(uint16_t));
    for (uint16_t i = 0; i < dec->m; i++) {
        for (uint16_t ptr = dec->row_ptr[i]; ptr < dec->row_ptr[i+1]; ptr++) {
            col_weights[ dec->col_idx[ptr] ]++;
        }
    }

    // Allocate memory and calculate threshold[j] = floor(col_weights[j]/2)
    dec->threshold = malloc(H_N * sizeof(uint8_t));
    for (uint16_t j = 0; j < dec->n; j++) {
        dec->threshold[j] = (uint8_t)(col_weights[j] / 2);
    }

    free(col_weights);
}


// with dec->n = total number of bits of codeword, dec->k = number of information bits.

int decode(BitFlipDecoder *dec,
                 const uint8_t *received_bytes, // input: n/8 bytes
                 uint8_t *info_bytes)          // output: k/8 bytes
{
    uint16_t m = dec->m;
    uint16_t n = dec->n;
    uint16_t k = n-m;
    uint16_t max_iter = dec->max_iter;

    // Number of bytes for input/output
    uint16_t n_bytes = n >> 3;   // assume n%8==0
    uint16_t k_bytes = k >> 3;   // assume k%8==0

    // 1) Unpack received_bytes into decoded bit array[0..n-1]
    uint8_t *decoded = malloc(n);
    if (!decoded) return -1;
    for (uint16_t j = 0; j < n; j++) {
        uint16_t byte_idx = j >> 3;
        uint8_t  bit_idx  = 7 - (j & 7);
        decoded[j] = (received_bytes[byte_idx] >> bit_idx) & 0x1;
    }

    // 2) Prepare memory for syndrome and unsat_counts
    uint8_t  *syndrome     = calloc(m,   sizeof(uint8_t ));
    uint16_t *unsat_counts = calloc(n,   sizeof(uint16_t));
    if (!syndrome || !unsat_counts) {
        free(decoded);
        free(syndrome);
        free(unsat_counts);
        return -2;
    }

    // Bit-flipping loop
    for (uint16_t iter = 0; iter < max_iter; iter++) {
        // Calculate syndrome = H * decoded (mod 2)
        for (uint16_t i = 0; i < m; i++) {
            uint8_t sum = 0;
            for (uint16_t ptr = dec->row_ptr[i]; ptr < dec->row_ptr[i+1]; ptr++) {
                sum ^= decoded[ dec->col_idx[ptr] ];
            }
            syndrome[i] = sum;
        }
        // Convergence test
        uint8_t any1 = 0;
        for (uint16_t i = 0; i < m; i++) {
            if (syndrome[i]) { any1 = 1; break; }
        }
        if (!any1) break;

        // Count unsatisfied checks per bit
        memset(unsat_counts, 0, n * sizeof(uint16_t));
        for (uint16_t i = 0; i < m; i++) {
            if (syndrome[i]) {
                for (uint16_t ptr = dec->row_ptr[i]; ptr < dec->row_ptr[i+1]; ptr++) {
                    unsat_counts[ dec->col_idx[ptr] ]++;
                }
            }
        }

        // Static-phase
        uint8_t did_flip = 0;
        for (uint16_t j = 0; j < n; j++) {
            if (unsat_counts[j] > dec->threshold[j]) {
                decoded[j] ^= 1;
                did_flip = 1;
            }
        }
        // Dynamic-phase
        if (!did_flip) {
            uint16_t max_unsat = 0;
            for (uint16_t j = 0; j < n; j++) {
                if (unsat_counts[j] > max_unsat) {
                    max_unsat = unsat_counts[j];
                }
            }
            for (uint16_t j = 0; j < n; j++) {
                if (unsat_counts[j] == max_unsat) {
                    decoded[j] ^= 1;
                    break;
                }
            }
        }
    }

    // 3) Pack the first k decoded bits[0..k-1] into info_bytes[0..k_bytes-1]
    memset(info_bytes, 0, k_bytes);
    for (uint16_t i = 0; i < k; i++) {
        if (decoded[i]) {
            uint16_t byte_idx = i >> 3;
            uint8_t  bit_idx  = 7 - (i & 7);
            info_bytes[byte_idx] |= (1 << bit_idx);
        }
    }

    // Free up memory
    free(decoded);
    free(syndrome);
    free(unsat_counts);

    return 0;
}

