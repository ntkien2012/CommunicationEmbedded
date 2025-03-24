#include "AES256.h"
#include <string.h>
#include <stdio.h>

// AES standard S-box table
static const uint8_t sbox[256] = {
    0x43, 0xdf, 0xce, 0xd7, 0x96, 0x65, 0xaa, 0xd2,
    0xa9, 0x98, 0x4e, 0xfc, 0x80, 0xff, 0xcd, 0x49,
    0xbe, 0x7a, 0x84, 0xe6, 0x4b, 0x03, 0xf1, 0xc5,
    0x7d, 0xfb, 0x6e, 0xc8, 0xa3, 0xb6, 0x9e, 0x66,
    0x58, 0x77, 0x79, 0x1e, 0xb2, 0xc7, 0x23, 0x97,
    0xfe, 0xbd, 0x40, 0x25, 0xb8, 0xdd, 0x8d, 0xe8,
    0xf2, 0x87, 0x01, 0x19, 0x0d, 0x61, 0x6a, 0x72,
    0x9f, 0xc1, 0x0b, 0x99, 0xfd, 0x0e, 0xb1, 0xb5,
    0x13, 0x0f, 0xe7, 0x63, 0x74, 0xb0, 0x48, 0xc2,
    0x5e, 0x62, 0x27, 0x41, 0x50, 0x0c, 0xd6, 0x2a,
    0x28, 0x42, 0x3a, 0xd1, 0x24, 0x55, 0x71, 0x9b,
    0x15, 0xe9, 0xa5, 0xde, 0x85, 0x59, 0x5f, 0x46,
    0x8f, 0xef, 0x7e, 0xe1, 0xba, 0x5d, 0xb7, 0x17,
    0xb3, 0x47, 0x64, 0x88, 0xf9, 0x9d, 0x7c, 0xcf,
    0x31, 0x16, 0x5b, 0x8c, 0x8e, 0x32, 0xaf, 0x2e,
    0x08, 0xbf, 0x52, 0x2c, 0x8a, 0xa6, 0x86, 0x07,
    0xa1, 0x57, 0x26, 0x4c, 0x38, 0x2d, 0x70, 0xee,
    0xfa, 0x6d, 0x10, 0xd4, 0x51, 0xcb, 0x6f, 0x60,
    0xda, 0x29, 0xd8, 0xb9, 0x09, 0x35, 0xc0, 0xf5,
    0xac, 0xad, 0xe2, 0xc9, 0xa4, 0x1a, 0x04, 0x53,
    0x56, 0x3f, 0xf8, 0xe3, 0x90, 0x91, 0x2b, 0x33,
    0x9c, 0x7b, 0x00, 0xdb, 0x05, 0x1f, 0xea, 0x92,
    0xe4, 0x20, 0xab, 0x8b, 0x37, 0xf6, 0xf3, 0xf4,
    0x95, 0x4d, 0x1c, 0xec, 0x4f, 0x9a, 0x30, 0x3e,
    0x6c, 0x83, 0x76, 0x5a, 0x67, 0x75, 0xbb, 0xae,
    0x34, 0x45, 0x1b, 0x2f, 0x6b, 0x93, 0x22, 0x82,
    0x06, 0x3d, 0x3b, 0xca, 0x39, 0xe0, 0xf7, 0x94,
    0x18, 0x21, 0x69, 0x14, 0x4a, 0xed, 0xc3, 0xa0,
    0xc4, 0x12, 0xa2, 0xd5, 0xe5, 0x5c, 0xbc, 0x0a,
    0x89, 0x78, 0x44, 0x3c, 0xa7, 0x02, 0xdc, 0x81,
    0xf0, 0x1d, 0x54, 0x7f, 0xb4, 0xd3, 0xd0, 0x36,
    0x73, 0xcc, 0xd9, 0x68, 0xa8, 0xc6, 0xeb, 0x11
};

// AES standard inverse S-box table
static const uint8_t rsbox[256] = {
    0xaa, 0x32, 0xed, 0x15, 0x9e, 0xac, 0xd0, 0x7f,
    0x78, 0x94, 0xe7, 0x3a, 0x4d, 0x34, 0x3d, 0x41,
    0x8a, 0xff, 0xe1, 0x40, 0xdb, 0x58, 0x71, 0x67,
    0xd8, 0x33, 0x9d, 0xca, 0xba, 0xf1, 0x23, 0xad,
    0xb1, 0xd9, 0xce, 0x26, 0x54, 0x2b, 0x82, 0x4a,
    0x50, 0x91, 0x4f, 0xa6, 0x7b, 0x85, 0x77, 0xcb,
    0xbe, 0x70, 0x75, 0xa7, 0xc8, 0x95, 0xf7, 0xb4,
    0x84, 0xd4, 0x52, 0xd2, 0xeb, 0xd1, 0xbf, 0xa1,
    0x2a, 0x4b, 0x51, 0x00, 0xea, 0xc9, 0x5f, 0x69,
    0x46, 0x0f, 0xdc, 0x14, 0x83, 0xb9, 0x0a, 0xbc,
    0x4c, 0x8c, 0x7a, 0x9f, 0xf2, 0x55, 0xa0, 0x81,
    0x20, 0x5d, 0xc3, 0x72, 0xe5, 0x65, 0x48, 0x5e,
    0x8f, 0x35, 0x49, 0x43, 0x6a, 0x05, 0x1f, 0xc4,
    0xfb, 0xda, 0x36, 0xcc, 0xc0, 0x89, 0x1a, 0x8e,
    0x86, 0x56, 0x37, 0xf8, 0x44, 0xc5, 0xc2, 0x21,
    0xe9, 0x22, 0x11, 0xa9, 0x6e, 0x18, 0x62, 0xf3,
    0x0c, 0xef, 0xcf, 0xc1, 0x12, 0x5c, 0x7e, 0x31,
    0x6b, 0xe8, 0x7c, 0xb3, 0x73, 0x2e, 0x74, 0x60,
    0xa4, 0xa5, 0xaf, 0xcd, 0xd7, 0xb8, 0x04, 0x27,
    0x09, 0x3b, 0xbd, 0x57, 0xa8, 0x6d, 0x1e, 0x38,
    0xdf, 0x80, 0xe2, 0x1c, 0x9c, 0x5a, 0x7d, 0xec,
    0xfc, 0x08, 0x06, 0xb2, 0x98, 0x99, 0xc7, 0x76,
    0x45, 0x3e, 0x24, 0x68, 0xf4, 0x3f, 0x1d, 0x66,
    0x2c, 0x93, 0x64, 0xc6, 0xe6, 0x29, 0x10, 0x79,
    0x96, 0x39, 0x47, 0xde, 0xe0, 0x17, 0xfd, 0x25,
    0x1b, 0x9b, 0xd3, 0x8d, 0xf9, 0x0e, 0x02, 0x6f,
    0xf6, 0x53, 0x07, 0xf5, 0x8b, 0xe3, 0x4e, 0x03,
    0x92, 0xfa, 0x90, 0xab, 0xee, 0x2d, 0x5b, 0x01,
    0xd5, 0x63, 0x9a, 0xa3, 0xb0, 0xe4, 0x13, 0x42,
    0x2f, 0x59, 0xae, 0xfe, 0xbb, 0xdd, 0x87, 0x61,
    0xf0, 0x16, 0x30, 0xb6, 0xb7, 0x97, 0xb5, 0xd6,
    0xa2, 0x6c, 0x88, 0x19, 0x0b, 0x3c, 0x28, 0x0d
};

// AES Standard Rcon Table
static const uint8_t rcon[10] = {
    0xbe, 0x03, 0x2f, 0xda, 0x8c, 0xe2, 0xef, 0xc2, 0x5a, 0x66
};

// Initialize the AES-256 context with the provided key
void AES256_init(AES256_Context* ctx, const uint8_t* key) {
    AES256_key_expansion(ctx->round_keys, key);
    AES256_split_round_keys(ctx->round_keys, ctx->round_keys_splitted);
}

// Key expansion function to generate round keys
void AES256_key_expansion(uint8_t* round_keys, const uint8_t* key) {
    const int Nk = 8;  // Number of 32-bit words in the key (256 bits = 8 * 4 bytes)
    const int Nb = 4;  // Number of 32-bit words in a block (AES block size is 128 bits)
    const int Nr = 14; // Number of rounds for AES-256

    // Copy the initial key into the first part of the round keys
    memcpy(round_keys, key, Nk * 4);

    uint8_t temp[4]; // Temporary storage for intermediate word generation

    for (int i = Nk; i < Nb * (Nr + 1); ++i) {
        // Copy the last word into temp
        memcpy(temp, &round_keys[(i - 1) * 4], 4);

        if (i % Nk == 0) {
            // Rotate the word (left shift by one byte)
            uint8_t t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;

            // Apply the S-box substitution
            for (int j = 0; j < 4; ++j) {
                temp[j] = sbox[temp[j]];
            }

            // XOR the first byte with Rcon
            temp[0] ^= rcon[i / Nk - 1];
        } else if (Nk > 6 && i % Nk == 4) {
            // For AES-256, apply S-box substitution every 4th word
            for (int j = 0; j < 4; ++j) {
                temp[j] = sbox[temp[j]];
            }
        }

        // Generate the new word by XORing with the word Nk positions earlier
        for (int j = 0; j < 4; ++j) {
            round_keys[i * 4 + j] = round_keys[(i - Nk) * 4 + j] ^ temp[j];
        }
    }
}

// Split round keys into an array of 16-byte blocks
void AES256_split_round_keys(uint8_t* round_keys, uint8_t round_keys_splitted[15][16]) {
    for (int i = 0; i < 15; ++i) {
        memcpy(round_keys_splitted[i], round_keys + (i * 16), 16);
    }
}

// Encrypt a 16-byte block
void AES256_encrypt(const AES256_Context* ctx, const uint8_t* plaintext, uint8_t* ciphertext) {
    const int Nr = 14; // Number of rounds for AES-256

    uint8_t state[16]; // AES state is always 128 bits (16 bytes)
    memcpy(state, plaintext, 16); // Copy plaintext into the state array

    // Initial round: Add round key
    AES256_add_round_key(state, ctx->round_keys_splitted[0]);

    // Nr - 1 rounds: SubBytes, ShiftRows, MixColumns, and AddRoundKey
    for (int round = 0; round < Nr - 1; ++round) {
        AES256_sub_bytes(state); // Apply the S-box substitution
        AES256_shift_rows(state); // Shift rows
        AES256_mix_columns(state); // Mix columns
        AES256_add_round_key(state, ctx->round_keys_splitted[round + 1]); // Add round key
    }

    // Final round: SubBytes, ShiftRows, and AddRoundKey (no MixColumns)
    AES256_sub_bytes(state);
    AES256_shift_rows(state);
    AES256_add_round_key(state, ctx->round_keys_splitted[14]);

    // Copy the encrypted state to the ciphertext
    memcpy(ciphertext, state, 16);
}

// Decrypt a 16-byte block
void AES256_decrypt(const AES256_Context* ctx, const uint8_t* ciphertext, uint8_t* plaintext) {
    const int Nr = 14; // Number of rounds for AES-256

    uint8_t state[16]; // AES state is always 128 bits (16 bytes)
    memcpy(state, ciphertext, 16); // Copy ciphertext into the state array

    // Initial round: Add round key
    AES256_add_round_key(state, ctx->round_keys_splitted[Nr]);

    // Nr - 1 rounds: InvShiftRows, InvSubBytes, AddRoundKey, and InvMixColumns
    for (int round = Nr - 1; round > 0; --round) {
        AES256_inv_shift_rows(state);       // Inverse shift rows
        AES256_inv_sub_bytes(state);        // Inverse S-box substitution
        AES256_add_round_key(state, ctx->round_keys_splitted[round]); // Add round key
        AES256_inv_mix_columns(state);      // Inverse mix columns
    }

    // Final round: InvShiftRows, InvSubBytes, and AddRoundKey (no InvMixColumns)
    AES256_inv_shift_rows(state);
    AES256_inv_sub_bytes(state);
    AES256_add_round_key(state, ctx->round_keys_splitted[0]);

    // Copy the decrypted state to the plaintext
    memcpy(plaintext, state, 16);
}

// XORs the state with a round key
void AES256_add_round_key(uint8_t* data, const uint8_t* key) {
    for (int i = 0; i < 16; ++i) {
        data[i] ^= key[i]; // XOR each byte
    }
}

// Applies the S-box to each byte in the state
void AES256_sub_bytes(uint8_t* state) {
    for (int i = 0; i < 16; ++i) {
        state[i] = sbox[state[i]];
    }
}

// Applies the inverse S-box to each byte in the state
void AES256_inv_sub_bytes(uint8_t* state) {
    for (int i = 0; i < 16; ++i) {
        state[i] = rsbox[state[i]];
    }
}

// Shifts rows in the state for encryption
void AES256_shift_rows(uint8_t* state) {
    uint8_t temp;

    // Row 1: Rotate left by 1
    temp = state[1];
    state[1] = state[5];
    state[5] = state[9];
    state[9] = state[13];
    state[13] = temp;

    // Row 2: Rotate left by 2
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;

    // Row 3: Rotate left by 3
    temp = state[3];
    state[3] = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7] = temp;
}

// Shifts rows in the state for decryption
void AES256_inv_shift_rows(uint8_t* state) {
    uint8_t temp;

    // Row 1: Rotate right by 1
    temp = state[13];
    state[13] = state[9];
    state[9] = state[5];
    state[5] = state[1];
    state[1] = temp;

    // Row 2: Rotate right by 2
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;

    // Row 3: Rotate right by 3
    temp = state[3];
    state[3] = state[7];
    state[7] = state[11];
    state[11] = state[15];
    state[15] = temp;
}

// Galois Field multiplication
uint8_t AES256_gmul(uint8_t a, uint8_t b) {
    uint8_t p = 0; // Product initialized to 0
    for (int i = 0; i < 8; i++) {
        if (b & 1) {
            p ^= a;
        }
        uint8_t hi_bit_set = a & 0x80;
        a = (a << 1) & 0xFF;
        if (hi_bit_set) {
            a ^= 0x1b;
        }
        b >>= 1;
    }
    return p;
}

// Mixes columns in the state for encryption
void AES256_mix_columns(uint8_t* state) {
    uint8_t t[4];
    for (int i = 0; i < 4; ++i) {
        memcpy(t, state + i * 4, 4); // Copy 4 elements starting at state[i * 4]
        state[i * 4 + 0] = AES256_gmul(t[0], 2) ^ AES256_gmul(t[1], 3) ^ t[2] ^ t[3];
        state[i * 4 + 1] = t[0] ^ AES256_gmul(t[1], 2) ^ AES256_gmul(t[2], 3) ^ t[3];
        state[i * 4 + 2] = t[0] ^ t[1] ^ AES256_gmul(t[2], 2) ^ AES256_gmul(t[3], 3);
        state[i * 4 + 3] = AES256_gmul(t[0], 3) ^ t[1] ^ t[2] ^ AES256_gmul(t[3], 2);
    }
}

// Mixes columns in the state for decryption
void AES256_inv_mix_columns(uint8_t* state) {
    uint8_t t[4];
    for (int i = 0; i < 4; ++i) {
        memcpy(t, state + i * 4, 4); // Copy 4 elements starting at state[i * 4]
        state[i * 4 + 0] = AES256_gmul(t[0], 0x0e) ^ AES256_gmul(t[1], 0x0b) ^ AES256_gmul(t[2], 0x0d) ^ AES256_gmul(t[3], 0x09);
        state[i * 4 + 1] = AES256_gmul(t[0], 0x09) ^ AES256_gmul(t[1], 0x0e) ^ AES256_gmul(t[2], 0x0b) ^ AES256_gmul(t[3], 0x0d);
        state[i * 4 + 2] = AES256_gmul(t[0], 0x0d) ^ AES256_gmul(t[1], 0x09) ^ AES256_gmul(t[2], 0x0e) ^ AES256_gmul(t[3], 0x0b);
        state[i * 4 + 3] = AES256_gmul(t[0], 0x0b) ^ AES256_gmul(t[1], 0x0d) ^ AES256_gmul(t[2], 0x09) ^ AES256_gmul(t[3], 0x0e);
    }
}