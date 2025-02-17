#ifndef AES256_HPP
#define AES256_HPP

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <vector>

class AES256{
public:
    AES256(const uint8_t *key);
    void encrypt(const uint8_t* plaintext, const uint8_t* ciphertext);
    void decrypt(const uint8_t* ciphertext, const uint8_t* plaintext);
    void key_expansion(uint8_t* round_keys, const uint8_t *key);

private:
    uint8_t round_keys[240];
    uint8_t round_keys_splitted[15][16];

    void add_round_key(uint8_t *state, uint8_t *key);
    void sub_bytes(uint8_t *state) const;
    void inv_sub_bytes(uint8_t *state) const;
    void shift_rows(uint8_t *state) const;
    void inv_shift_rows(uint8_t *state) const;
    void mix_columns(uint8_t *state) const;
    void inv_mix_columns(uint8_t *state) const;
    static uint8_t gmul(uint8_t a, uint8_t b);

};

#endif //AES256_HPP