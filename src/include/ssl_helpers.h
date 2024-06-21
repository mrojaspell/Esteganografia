
#ifndef SSL_HELPERS
#define SSL_HELPERS
#include <openssl/evp.h>
#include <string.h>
#include <stdint.h>
#include "enums.h"
#include "print_error.h"
#include "status_codes.h"

typedef const EVP_CIPHER *(*cypher_strategy)(void);




typedef struct password_metadata
{
    char *password;
    uint8_t *key;
    uint8_t *init_vector;
    cypher_strategy cypher;
} password_metadata;

int initialize_password_metadata(password_metadata * password_metadata,encryption_alg encription_alg, block_chaining_mode block_chaining_mode);
int decrypt_payload(uint8_t * encrypted_input, uint32_t encrypted_length, uint8_t * decrypted_out,password_metadata * password_metadata);
int encrypt_payload(uint8_t * decrypted_input, uint32_t input_len, uint8_t** encrypted_output, uint32_t *output_size, password_metadata * password_metadata);
#endif