#include <stddef.h>
#include <strings.h>

#include "include/enums.h"

program_mode get_program_mode(char *mode_str)
{
    if (!strcasecmp(mode_str, "-embed"))
        return EMBED_MODE;
    if (!strcasecmp(mode_str, "-extract"))
        return EXTRACT_MODE;

    return UNSPECIFIED_MODE;
}

steg_alg get_steg_alg(char *steg_str)
{
    if (!strcasecmp(steg_str, "LSB1"))
        return LSB1;
    if (!strcasecmp(steg_str, "LSB4"))
        return LSB4;
    if (!strcasecmp(steg_str, "LSBI"))
        return LSBI;

    return UNSPECIFIED_STEG;
}

encryption_alg get_encryption_alg(char *encryption_str)
{
    if (!strcasecmp(encryption_str, "AES128"))
        return AES128;
    if (!strcasecmp(encryption_str, "AES192"))
        return AES192;
    if (!strcasecmp(encryption_str, "AES256"))
        return AES256;
    if (!strcasecmp(encryption_str, "DES"))
        return DES;
    return AES128;
}

block_chaining_mode get_chaining_mode(char *chaining_str)
{
    if (!strcasecmp(chaining_str, "CBC"))
        return CBC;
    if (!strcasecmp(chaining_str, "ECB"))
        return ECB;
    if (!strcasecmp(chaining_str, "CFB"))
        return CFB;
    if (!strcasecmp(chaining_str, "OFB"))
        return OFB;
    return CBC;
}