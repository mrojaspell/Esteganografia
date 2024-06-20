#ifndef EMBED_LSBN_H
#define EMBED_LSBN_H
#include "status_codes.h"
#include "enums.h"


status_code embed_lsbn(unsigned char n, char * in_file_path, char * p_file_path, char * out_file_path, encryption_alg encryption, block_chaining_mode chaining, char * password);

#endif