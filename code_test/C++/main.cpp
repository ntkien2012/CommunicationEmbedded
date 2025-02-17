#include "AES_256.hpp"
#include <cstring>
#include <iostream>

int main (){
    uint8_t key[32] = {
        0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
        0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
        0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
        0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
    };

    uint8_t plaintext[16] = {
        0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
        0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
    };
    uint8_t ciphertext[16] = {0};
    uint8_t decryptedtext[16] = {0};   

    AES256 aes(key);

    std::cout << "plaintext: ";
    for (int i=0; i<16; ++i){
        printf("%02x ", plaintext[i]);
    }
    std::cout << std::endl;

    aes.encrypt(plaintext, ciphertext);
    std::cout << "Ciphertext: ";
    for (int i=0; i<16; ++i){
        printf("%02x ", ciphertext[i]);
    }
    std::cout << std::endl;

    aes.decrypt(ciphertext, decryptedtext);
    std::cout << "decryptedtext: ";
    for (int i=0; i<16; ++i){
        printf("%02x ", decryptedtext[i]);
    }
    std::cout << std::endl;

    return 0;
}
