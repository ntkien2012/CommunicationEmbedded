#include "AES_256.hpp"
#include <cstring>

// AES S-box
const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

const uint8_t rsbox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

// Rcon table
static const uint8_t rcon[10] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36
};

// Constructor: Initializes the round keys using the provided key
AES256::AES256(const uint8_t *key) {
    key_expansion(round_keys, key);
}

void AES256::key_expansion (uint8_t *round_keys, const uint8_t *key){
    const int Nk = 8; // Number of 32-bit words in the key
    const int Nb = 4; // Number of 32-bit words in a block
    const int Nr = 14; // Number of rounds for AES-256

    // Copy the initial key into the first part of the round keys
    std::memcpy(round_keys, key, Nk * 4);

    uint8_t temp[4];

    for (int i = Nk; i < Nb * (Nr + 1); ++i){
        // copy the last word into temp
        std::memcpy(temp, &round_keys[(i-1)*4], 4);
        
        if (i%Nk == 0){
            //rotate the word (left shift by one byte)
            uint8_t t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;

            //apply the S-box substitution
            for (int j=0; j<4; ++j){
                temp[j] = sbox[temp[j]];
            }

            //XOR the first byte with Rcon
            temp[0] ^= rcon[i/Nk - 1];
        } else if (Nk > 6 && i%Nk == 4){
            //for AES_256, apply S-box substitution every 4th word
            for (int j = 0; j < 4; ++j){
                temp[j] = sbox[temp[j]];
            }
        }

        //Generate the new word by XORing with the word Nk positions earlier
        for (int j = 0; j < 4; ++j){
            round_keys[i*4 + j] = round_keys[(i - Nk)*4 + j] ^ temp[j];
        }
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~ function split round keys ~~~~~~~~~~~~~~~ //
void split_round_keys (uint8_t *round_keys, uint8_t *round_keys_splitted){
    const int num_rounds = 15;
    const int key_size = 16;

    for (int i = 0; i < num_rounds; ++i){
        // Copy 16 byte from round_key to round_keys_splitted[i]
        memcpy (&round_keys_splitted[i*key_size], &round_keys[i*key_size], key_size);
    }
}

void AES256::encrypt(const uint8_t* plaintext, const uint8_t* ciphertext){
    //const int Nb = 4;     // Number of columns (32-bit words) in a state (fixed at 4 for AES)
    const int Nr = 14;      // Number of rounds for AES_256

    uint8_t state[16];      // AES state is always 128 bits (16 bytes)
    memcpy(state, plaintext, 16); // copy plaintext into the state array
    split_round_keys(round_keys, reinterpret_cast<uint8_t*>(round_keys_splitted));

    // Initial round: Add round key
    add_round_key(state, round_keys_splitted[0]);
    // Nr - 1 rounds: SubBytes, ShiftRows, MixColumns and AddRoundKey
    for (int round = 0; round < Nr - 1; ++round){
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, round_keys_splitted[round+1]); 
    }

    // final round: SubBytes, ShiftRows and AddRoundKey (not MixColumns)
    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, round_keys_splitted[14]);

    // Copy the encrypted state to the ciphertext
    memcpy(const_cast<uint8_t*>(ciphertext), state, 16);
}

void AES256::decrypt(const uint8_t* ciphertext, const uint8_t* plaintext){
    const int Nr = 14;

    uint8_t state[16];
    memcpy(state, ciphertext, 16);
    split_round_keys(round_keys, reinterpret_cast<uint8_t*>(round_keys_splitted));

    // intitial round: Add round key
    add_round_key(state, round_keys_splitted[Nr]);

    // Nr - 1 rounds: InvShiftRows, IncSubBytes, AddRoundKey and InvMixColumns
    for (int round = Nr - 1; round > 0; --round){
        inv_shift_rows(state);
        inv_sub_bytes(state);
        add_round_key(state, round_keys_splitted[round]);
        inv_mix_columns(state);
    }

    // Final round: InvShiftRows, InvSubBytes and AddRoundKey (not InvMixColumns)
    inv_shift_rows(state);
    inv_sub_bytes(state);
    add_round_key(state, round_keys_splitted[0]);

    // copy the decrypted state to the plaintext
    memcpy (const_cast<uint8_t*>(plaintext), state, 16);
}


//XORs the state with a round key
void AES256::add_round_key(uint8_t *data, uint8_t *key){
    for (int i = 0; i < 16; ++i){
        data[i] ^= key[i]; //XOR each byte
    }
}

// Applies the S-box to each byte in the state
void AES256::sub_bytes(uint8_t *state) const {
    for (int i = 0; i < 16; ++i) {
        state[i] = sbox[state[i]];
    }
}

// Applies the S-box to each byte in the state
void AES256::inv_sub_bytes(uint8_t *state) const {
    for (int i = 0; i < 16; ++i){
        state[i] = rsbox[state[i]];
    }
}

// shift rows in the state for encryption
void AES256::shift_rows(uint8_t *state) const {
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
void AES256::inv_shift_rows(uint8_t *state) const {
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

uint8_t AES256::gmul(uint8_t a, uint8_t b) {
    // Galois Field multiplication of a and b
    uint8_t p = 0; // Product initialized to 0
    for (int i = 0; i < 8; i++) {
        // If the least significant bit of b is set, XOR p with a
        if (b & 1) {
            p ^= a;
        }
        // Check if the high bit of a is set
        uint8_t hi_bit_set = a & 0x80;
        // Left shift a and keep it within 8 bits
        a = (a << 1) & 0xFF;
        // If the high bit was set, XOR a with the irreducible polynomial 0x1b
        if (hi_bit_set) {
            a ^= 0x1b;
        }
        // Right shift b
        b >>= 1;
    }
    return p;
}

// Mixes columns in the state for encryption
void AES256::mix_columns(uint8_t *state) const {
    uint8_t t[4];
    for (int i = 0; i < 4; ++i) {
        std::memcpy(t, state + i * 4, 4); // Copy 4 elements starting at state[i * 4]
        // Perform the mix column transformation
        state[i * 4 + 0] = gmul(t[0], 2) ^ gmul(t[1], 3) ^ t[2] ^ t[3];
        state[i * 4 + 1] = t[0] ^ gmul(t[1], 2) ^ gmul(t[2], 3) ^ t[3];
        state[i * 4 + 2] = t[0] ^ t[1] ^ gmul(t[2], 2) ^ gmul(t[3], 3);
        state[i * 4 + 3] = gmul(t[0], 3) ^ t[1] ^ t[2] ^ gmul(t[3], 2);
    }
}

// Mixes columns in the state for decryption
void AES256::inv_mix_columns(uint8_t *state) const {
    uint8_t t[4];
    for (int i = 0; i < 4; ++i) {
        std::memcpy(t, state + i * 4, 4); // Copy 4 elements starting at state[i * 4]
        // Perform the mix column transformation
        state[i * 4 + 0] = gmul(t[0], 0x0e) ^ gmul(t[1], 0x0b) ^ gmul(t[2], 0x0d) ^ gmul(t[3], 0x09);
        state[i * 4 + 1] = gmul(t[0], 0x09) ^ gmul(t[1], 0x0e) ^ gmul(t[2], 0x0b) ^ gmul(t[3], 0x0d);
        state[i * 4 + 2] = gmul(t[0], 0x0d) ^ gmul(t[1], 0x09) ^ gmul(t[2], 0x0e) ^ gmul(t[3], 0x0b);
        state[i * 4 + 3] = gmul(t[0], 0x0b) ^ gmul(t[1], 0x0d) ^ gmul(t[2], 0x09) ^ gmul(t[3], 0x0e);
    }
}