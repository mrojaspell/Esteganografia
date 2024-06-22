#ifndef LSBI_INVERT_PATTERNS_H
#define LSBI_INVERT_PATTERNS_H

#include <stdint.h>
#include <stdio.h>

#include "status_codes.h"

status_code lsbi_invert_patterns(FILE * p_file, FILE * out_file, uint32_t in_file_size);

#endif