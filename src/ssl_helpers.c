//to .h

#include "ssl_helpers.h"

//end .h

int KEY_SIZE[] = {16, 24, 32, 8};
int BLOCK_SIZE[] = {16, 16, 16, 8};
cypher_strategy cypher_strategies[4][4] = {
    {EVP_aes_128_cbc, EVP_aes_128_ecb, EVP_aes_128_cfb, EVP_aes_128_ofb},
    {EVP_aes_192_cbc, EVP_aes_192_ecb, EVP_aes_192_cfb, EVP_aes_192_ofb},
    {EVP_aes_256_cbc, EVP_aes_256_ecb, EVP_aes_256_cfb, EVP_aes_256_ofb},
    {EVP_des_cbc, EVP_des_ecb, EVP_des_cfb, EVP_des_ofb}
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

    //initialize password initialization vector
    password_metadata->init_vector = malloc(BLOCK_SIZE[encription_alg]);
    if (password_metadata->init_vector == NULL) {
        print_error("Not enough memory to allocate iv");
        return MEMORY_ERROR;
    }

    password_metadata->key = malloc(KEY_SIZE[encription_alg]);
    if (password_metadata->key == NULL) {
        print_error("Not enough memory to allocate key");
        return MEMORY_ERROR;
    }

    EVP_CIPHER* result = (password_metadata->cypher)();
    EVP_BytesToKey(result, EVP_sha256(), (const unsigned char*)0,
                   (const unsigned char*)password_metadata->password,
                   strlen(password_metadata->password),
                   1,
                   password_metadata->key,
                   password_metadata->init_vector);

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
    EVP_CIPHER_CTX* ctx;
    ctx = EVP_CIPHER_CTX_new();

    if (!(ctx)) {
        print_error("Could not generate cypher context");
        return ENCRYPTION_ERROR;
    }

    EVP_CIPHER* result = (password_metadata->cypher)();
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
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        printf("Could not instanciate cypher context");
        return -1;
    }

    EVP_CIPHER* result = (password_metadata->cypher)();
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
