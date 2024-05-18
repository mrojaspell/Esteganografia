#include <stddef.h>
#include <string.h>

#include "include/enums.h"

program_mode get_program_mode(char * mode_str){
    if (!strcmp(mode_str, "-embed"))
        return EMBED_MODE;
    else if (!strcmp(mode_str, "-extract"))
        return EXTRACT_MODE;
    
    return UNSPECIFIED_MODE;
}


steg_alg get_steg_alg(char * steg_str){
    if (!strcmp(steg_str, "LSB1"))
        return LSB1;
    else if (!strcmp(steg_str, "LSB4"))
        return LSB4;
    else if (!strcmp(steg_str, "LSBI"))
        return LSBI;
    
    return UNSPECIFIED_STEG;
}

encryption_alg get_encryption_alg(char * encryption_str){
    if (!strcmp(encryption_str, "AES128"))
        return AES128;
    else if (!strcmp(encryption_str, "AES192"))
        return AES192;
    else if (!strcmp(encryption_str, "AES256"))
        return AES256;
    else if (!strcmp(encryption_str, "DES"))
        return DES;
    
    return UNSPECIFIED_ENC;
}

block_chaining_mode get_chaining_mode(char * chaining_str){
    if (!strcmp(chaining_str, "CBC"))
        return CBC;
    else if (!strcmp(chaining_str, "ECB"))
        return ECB;
    else if (!strcmp(chaining_str, "CFB"))
        return CFB;
    else if (!strcmp(chaining_str, "OFB"))
        return OFB;
    
    return UNSPECIFIED_CHAIN;
}