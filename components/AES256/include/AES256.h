#ifndef AES256_H
#define AES256_H

#include <stdint.h>

#define AES_BLOCK_SIZE 16

#ifdef __cplusplus
extern "C" {
#endif

// AES-256 context structure
typedef struct {
    uint8_t round_keys[240];       // 15 rounds * 16 bytes (AES-256 requires 240 bytes for round keys)
    uint8_t round_keys_splitted[15][16]; // Split round keys for easier access
} AES256_Context;

// Function declarations
void AES256_init(AES256_Context* ctx, const uint8_t* key);
void AES256_encrypt(const AES256_Context* ctx, const uint8_t* plaintext, uint8_t* ciphertext);
void AES256_decrypt(const AES256_Context* ctx, const uint8_t* ciphertext, uint8_t* plaintext);

void AES256_key_expansion(uint8_t* round_keys, const uint8_t* key);
void AES256_split_round_keys(uint8_t* round_keys, uint8_t round_keys_splitted[15][16]);
void AES256_add_round_key(uint8_t* data, const uint8_t* key);
void AES256_sub_bytes(uint8_t* state);
void AES256_inv_sub_bytes(uint8_t* state);
void AES256_shift_rows(uint8_t* state);
void AES256_inv_shift_rows(uint8_t* state);
uint8_t AES256_gmul(uint8_t a, uint8_t b);
void AES256_mix_columns(uint8_t* state);
void AES256_inv_mix_columns(uint8_t* state);

#ifdef __cplusplus
}
#endif

#endif