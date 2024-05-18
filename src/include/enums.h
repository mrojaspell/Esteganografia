#ifndef ENUM_H
#define ENUM_H
#include <stdbool.h>

// Program Mode
typedef enum program_mode {
    UNSPECIFIED_MODE, // no default value
    EMBED_MODE,
    EXTRACT_MODE
} program_mode;
program_mode get_program_mode(char * mode_str);

// Steg Mode
typedef enum steg_alg {
    UNSPECIFIED_STEG, // no default value
    LSB1,
    LSB4,
    LSBI
} steg_alg;
steg_alg get_steg_alg(char * steg_str);

// Encryption Mode
typedef enum encryption_alg {
    AES128, // default
    AES192,
    AES256,
    DES,
    UNSPECIFIED_ENC
} encryption_alg;
encryption_alg get_encryption_alg(char * encryption_str);

// Block Chaining Mode
typedef enum block_chaining_mode {
    CBC, // default
    ECB,
    CFB,
    OFB,
    UNSPECIFIED_CHAIN
} block_chaining_mode;
block_chaining_mode get_chaining_mode(char * chaining_str);


#endif