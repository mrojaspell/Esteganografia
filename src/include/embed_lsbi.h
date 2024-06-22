#ifndef EMBED_LSBI_H
#define EMBED_LSBI_H
#include "status_codes.h"
#include "ssl_helpers.h"

status_code embed_lsbi(unsigned char n, char* in_file_path, char* p_file_path, char* out_file_path, encryption_alg encryption, block_chaining_mode chaining, char *password);
#endif //EMBED_LSBI_H
