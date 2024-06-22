#ifndef EXTRACT_LSBI_H
#define EXTRACT_LSBI_H
#include "status_codes.h"
#include "ssl_helpers.h"
#include "enums.h"

status_code extract_lsbi(const char* p_bmp, const char* out_file_path, encryption_alg encryption, block_chaining_mode chaining, char * password);

#endif //EXTRACT_LSBi_H
