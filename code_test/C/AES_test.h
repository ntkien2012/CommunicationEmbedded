#ifndef AES_TEST_H
#define AES_TEST_H

#include <stdint.h>

#define AES_BLOCK_SIZE 16

typedef struct {
    uint8_t round_keys[240];
} AES256;

void key_expansion(uint8_t *round_keys, const uint8_t *key);
void encrypt(const uint8_t *plaintext, uint8_t *ciphertext, const uint8_t *round_keys);
void decrypt(const uint8_t *ciphertext, uint8_t *plaintext, const uint8_t *round_keys);
void add_round_key(uint8_t *state, const uint8_t *round_key);
void sub_bytes(uint8_t *state);
void inv_sub_bytes(uint8_t *state);
void shift_rows(uint8_t *state);
void inv_shift_rows(uint8_t *state);
void mix_columns(uint8_t *state);
void inv_mix_columns(uint8_t *state);

void split_round_keys (uint8_t *round_keys, uint8_t *round_keys_splitted);
static uint8_t gmul(uint8_t a, uint8_t b);

#endif // AES_TEST_H
