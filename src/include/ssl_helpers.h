
#ifndef SSL_HELPERS
#define SSL_HELPERS
#include <openssl/evp.h>
#include <string.h>
#include <stdint.h>
#include "enums.h"

typedef const EVP_CIPHER *(*cypher_strategy)(void);

/// @brief Specifically designet to match enum numbers
static cypher_strategy cypher_strategies[4][4] = {{EVP_aes_128_ecb, EVP_aes_128_cfb, EVP_aes_128_ofb, EVP_aes_128_cbc},
                                     {EVP_aes_192_ecb, EVP_aes_192_cfb, EVP_aes_192_ofb, EVP_aes_192_cbc},
                                     {EVP_aes_256_ecb, EVP_aes_128_cfb, EVP_aes_256_ofb, EVP_aes_256_cbc},
                                     {EVP_des_ecb, EVP_des_cfb, EVP_des_ofb, EVP_des_cbc}};


typedef struct password_metadata
{
    char *password;
    uint8_t *key;
    uint8_t *init_vector;
    cypher_strategy cypher;
} password_metadata;

int initialize_password_metadata(password_metadata * password_metadata,encryption_alg encription_alg, block_chaining_mode block_chaining_mode);
int decrypt_payload(uint8_t * encrypted_input, uint32_t encrypted_length, uint8_t * decrypted_out,password_metadata * password_metadata);
int encrypt_payload(uint8_t * decrypted_input, uint32_t input_len,uint8_t* encrypted_output,password_metadata * password_metadata);
#endif