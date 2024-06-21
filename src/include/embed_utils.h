#ifndef EMBED_UTILS_H
#define EMBED_UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "status_codes.h"
#include "colors.h"

status_code copy_from_file_to_file(FILE* copy_from, FILE* copy_to, const unsigned int len);
status_code copy_rest_of_file(FILE* copy_from, FILE* copy_to);

status_code embed_file_lsbni(const unsigned int n, FILE* p_file, FILE* out_file, FILE* in_file, bool lsbi, color * current_color);
status_code embed_number_lsbni(const unsigned int n, FILE* p_file, FILE* out_file, uint32_t number_to_embed, bool lsbi, color * current_color);
status_code embed_bytes_lsbni(const unsigned int n, FILE* p_file, FILE* out_file, const uint8_t* bytes_to_embed, unsigned int len, bool lsbi, color * current_color);

#endif