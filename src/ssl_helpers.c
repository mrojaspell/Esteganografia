


//to .h

#include "ssl_helpers.h"

//end .h

int KEY_SIZE[] = {16, 24, 32, 8};
int BLOCK_SIZE[] = {16, 16, 16, 8};



/// @brief this function initializes password metadata struct, however the struct must have a password string
/// @param password_metadata 
/// @param encription_alg required from the encryption algorithm enum
/// @param block_chaining_mode required from the block chaining enum 
/// @return returns 0 on success, 1 on errror
int initialize_password_metadata(password_metadata * password_metadata,encryption_alg encription_alg, block_chaining_mode block_chaining_mode)
{
    if (password_metadata->password == NULL)
    {
        printf("Password cannot be null, must be provided");
        return -1;
    }
    if (encription_alg == UNSPECIFIED_ENC){
        printf("Encryption mode unspecified");
        return -1;
    }
    if (block_chaining_mode == UNSPECIFIED_CHAIN)
    {
        printf("Encryption chaining mode unspecified");
        return -1;
    }
    password_metadata->cypher = cypher_strategies[encription_alg][block_chaining_mode];

    //initialize password initialization vector
    password_metadata->init_vector = malloc(BLOCK_SIZE[encription_alg]);
    if (password_metadata->init_vector == NULL)
    {
        printf("Not enough memory to allocate iv");
        return -1;
    }

    password_metadata->key = malloc(KEY_SIZE[encription_alg]);
    if (password_metadata->key == NULL)
    {
        printf("Not enough memory to allocate key");
        return -1;
    }

    EVP_CIPHER * result = (password_metadata->cypher)();
    EVP_BytesToKey(result,EVP_sha256(), (const unsigned char *) 0,
    (const unsigned char *)password_metadata->password,
    strlen(password_metadata->password),
    1,
    password_metadata->key,
    password_metadata->init_vector);


}


/// @brief this function encrypts using IV and password struct
/// @param decrypted_input input plain text
/// @param input_len input length
/// @param encrypted_output text to output
/// @param password_metadata metadata relevant for encription
/// @return lenght of cyphered text
int encrypt_payload(uint8_t * decrypted_input, uint32_t input_len,uint8_t* encrypted_output,password_metadata * password_metadata)
{
    EVP_CIPHER_CTX * ctx;

    if (!(ctx = EVP_CIPHER_CTX_new()))
    {
        printf("Could not instanciate cypher context");
        return -1;
    }

    EVP_CIPHER* result = (password_metadata->cypher)();

    if (EVP_EncryptInit_ex(ctx,result,NULL,password_metadata->key,password_metadata->init_vector) != 1)
    {
        printf("Could not initialize encription");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    int length;
    if (EVP_EncryptUpdate(ctx,encrypted_output,&length,decrypted_input,input_len) != 1)
    {
        printf("Error during input encryption");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    int cypher_length = length;
    
    if (EVP_EncryptFinal_ex(ctx,encrypted_output + cypher_length,&length))
    {
        printf("Error during encryption");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
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
int decrypt_payload(uint8_t * encrypted_input, uint32_t encrypted_length, uint8_t * decrypted_out,password_metadata * password_metadata)
{
    EVP_CIPHER_CTX * ctx;
    if (!(ctx = EVP_CIPHER_CTX_new()))
    {
        printf("Could not instanciate cypher context");
        return -1;
    }

    EVP_CIPHER* result = (password_metadata->cypher)();
    if (EVP_DecryptInit_ex(ctx, result,NULL,password_metadata->key,password_metadata->init_vector) != 1)
    {
        printf("Could not instanciate decryption resources");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    int lenght;
    if (EVP_DecryptUpdate(ctx,decrypted_out,&lenght,encrypted_input,encrypted_length) != 1)
    {
        printf("Could not decrypt content");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    int decrypted_length = lenght;
    if (EVP_DecryptFinal_ex(ctx,decrypted_out + decrypted_length,&lenght))
    {
        printf("Could not decrypt content");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    decrypted_length +=  lenght;
    EVP_CIPHER_CTX_free(ctx);
    return decrypted_length;


}