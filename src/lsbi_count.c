#include "lsbi_count.h"

#include <stdio.h>
#include "get_file_size.h"

status_code count_lsbi_changes_embedding(FILE *p_file, FILE *out_file, LSBI_count count[4]) {
    status_code exit_code = SUCCESS;
    rewind(p_file);
    rewind(out_file);

    for (int i = 0; i < 4; i++) {
        count[i].changed = 0;
        count[i].total = 0;
    }

    // TODO: Skip headers using functions in ssldevelopment branch

    // Skip LSBI pattern
    if (fseek(p_file, 4, SEEK_CUR)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    if (fseek(out_file, 4, SEEK_CUR)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    uint8_t p_byte, out_byte;
    // BMP files save data in BGR format, and in LSBI we need to skip the red byte
    int byte_count = 1;
    while (fread(&p_byte, 1, 1, p_file) == 1) {
        if (ferror(p_file)) {
            exit_code = FILE_READ_ERROR;
            goto finally;
        }

        if (fread(&out_byte, 1, 1, out_file) != 1) {
            exit_code = FILE_READ_ERROR;
            goto finally;
        }

        // Skip bytes used for red pixels
        if (byte_count % 3 == 0) {
            byte_count = 1;
            continue;
        }

        byte_count++;

        const uint8_t pattern = (p_byte >> 1) & 3; // 3 is 0b11
        count[pattern].total++;
        if ((p_byte & 0x01) != (out_byte & 0x01)) {
            count[pattern].changed++;
        }
    }

    if (!feof(p_file) || !feof(out_file)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

finally:
    return exit_code;
}
