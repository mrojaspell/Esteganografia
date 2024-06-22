#ifndef LSBI_COUNT_H
#define LSBI_COUNT_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "status_codes.h"

typedef struct LSBI_count {
    uint64_t total; // Total with this pattern in the original file
    uint64_t changed; // Total changed
    bool should_invert; // If we should invert the last bit for each byte in the pattern
} LSBI_count;

status_code count_lsbi_changes_embedding(FILE* p_file, FILE *out_file, LSBI_count count[4], uint32_t in_file_size);

#endif //LSBI_COUNT_H
