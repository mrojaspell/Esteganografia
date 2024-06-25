//to .h

#include "ssl_helpers.h"

//end .h

cypher_strategy cypher_strategies[4][4] = {
    {EVP_aes_128_cbc, EVP_aes_128_ecb, EVP_aes_128_cfb, EVP_aes_128_ofb},
    {EVP_aes_192_cbc, EVP_aes_192_ecb, EVP_aes_192_cfb, EVP_aes_192_ofb},
    {EVP_aes_256_cbc, EVP_aes_256_ecb, EVP_aes_256_cfb, EVP_aes_256_ofb},
    {EVP_des_ede3_cbc, EVP_des_ede3_ecb, EVP_des_ede3_cfb, EVP_des_ede3_ofb}
};


/// @brief this function initializes password metadata struct, however the struct must have a password string
/// @param password_metadata 
/// @param encription_alg required from the encryption algorithm enum
/// @param block_chaining_mode required from the block chaining enum 
/// @return returns 0 on success, 1 on errror
int initialize_password_metadata(password_metadata* password_metadata, encryption_alg encription_alg,
                                 block_chaining_mode block_chaining_mode) {
    if (password_metadata->password == NULL) {
        print_error("Password cannot be null, must be provided");
        return INVALID_PASSWORD;
    }

    password_metadata->cypher = cypher_strategies[encription_alg][block_chaining_mode];

    const int keylen = EVP_CIPHER_key_length(password_metadata->cypher());
    const int ivlen = EVP_CIPHER_iv_length(password_metadata->cypher());

    unsigned char* key_iv_pair = malloc(keylen + ivlen);
    if (key_iv_pair == NULL) {
        print_error("Not enough memory to allocate key and IV");
        return MEMORY_ERROR;
    }

    //initialize password initialization vector
    password_metadata->init_vector = malloc(ivlen);
    if (password_metadata->init_vector == NULL) {
        print_error("Not enough memory to allocate iv");
        return MEMORY_ERROR;
    }

    //initialize password key vector
    password_metadata->key = malloc(keylen);
    if (password_metadata->key == NULL) {
        print_error("Not enogh memory to allocate key");
        return MEMORY_ERROR;
    }

    const unsigned char salt[8] = {0};

    if (!PKCS5_PBKDF2_HMAC(password_metadata->password,
                           strlen(password_metadata->password),
                           salt,
                           sizeof(salt),
                           10000,
                           EVP_sha256(),
                           keylen + ivlen,
                           key_iv_pair)) {
        print_error("Key derivation failed");
        free(key_iv_pair);
        return KEY_DERIVATION_ERROR;
    }

    memcpy(password_metadata->key, key_iv_pair, keylen);
    memcpy(password_metadata->init_vector, key_iv_pair + keylen, ivlen);

    free(key_iv_pair);

    return SUCCESS;
}


/// @brief this function encrypts using IV and password struct
/// @param decrypted_input input plain text
/// @param input_len input length
/// @param encrypted_output text to output
/// @param password_metadata metadata relevant for encription
/// @return lenght of cyphered text
int encrypt_payload(uint8_t* decrypted_input, uint32_t input_len, uint8_t** encrypted_output, uint32_t* output_size,
                    password_metadata* password_metadata) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (ctx == NULL) {
        print_error("Could not generate cypher context");
        return ENCRYPTION_ERROR;
    }

    const EVP_CIPHER* result = (password_metadata->cypher)();
    *output_size = EVP_CIPHER_block_size(result) + input_len;

    *encrypted_output = malloc(*output_size);

    if (EVP_EncryptInit_ex(ctx, result, NULL, password_metadata->key, password_metadata->init_vector) != 1) {
        print_error("Could not start encryption");
        EVP_CIPHER_CTX_free(ctx);
        return ENCRYPTION_ERROR;
    }
    int length;
    if (EVP_EncryptUpdate(ctx, *encrypted_output, &length, decrypted_input, input_len) != 1) {
        print_error("Error encrypting input");
        EVP_CIPHER_CTX_free(ctx);
        return ENCRYPTION_ERROR;
    }
    int cypher_length = length;

    if (EVP_EncryptFinal_ex(ctx, (*encrypted_output) + cypher_length, &length) != 1) {
        print_error("Error finalizing encrypted input\n");
        EVP_CIPHER_CTX_free(ctx);
        return ENCRYPTION_ERROR;
    }

    cypher_length += length;
    EVP_CIPHER_CTX_free(ctx);
    return cypher_length;
}


/// @brief decrypts the provided payload, metadata for decryption is needed
/// @param encrypted_input 
/// @param encrypted_length 
/// @param decrypted_out 
/// @param password_metadata 
/// @return the length of the decrypted payload
int decrypt_payload(uint8_t* encrypted_input, uint32_t encrypted_length, uint8_t* decrypted_out,
                    password_metadata* password_metadata) {
    EVP_CIPHER_CTX* ctx;
    if ((ctx = EVP_CIPHER_CTX_new()) == NULL) {
        printf("Could not instanciate cypher context");
        return -1;
    }

    const EVP_CIPHER* result = (password_metadata->cypher)();
    if (EVP_DecryptInit_ex(ctx, result,NULL, password_metadata->key, password_metadata->init_vector) != 1) {
        printf("Could not instanciate decryption resources");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    int lenght;
    if (EVP_DecryptUpdate(ctx, decrypted_out, &lenght, encrypted_input, encrypted_length) != 1) {
        printf("Could not decrypt content");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    int decrypted_length = lenght;
    if (EVP_DecryptFinal_ex(ctx, decrypted_out + decrypted_length, &lenght) != 1) {
        printf("Could not decrypt content");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    EVP_CIPHER_CTX_free(ctx);
    return decrypted_length;
}
