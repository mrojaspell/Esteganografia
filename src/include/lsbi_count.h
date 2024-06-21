#ifndef LSBI_COUNT_H
#define LSBI_COUNT_H

#include <stdint.h>
#include <stdio.h>

#include "status_codes.h"

typedef struct LSBI_count {
    uint64_t total; // Total with this pattern in the original file
    uint64_t changed; // Total changed
} LSBI_count;

status_code count_lsbi_changes_embedding(FILE* p_file, FILE *out_file, LSBI_count count[4]);

#endif //LSBI_COUNT_H
