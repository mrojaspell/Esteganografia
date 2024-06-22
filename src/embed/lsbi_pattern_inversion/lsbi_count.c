#include "lsbi_count.h"
#include "colors.h"
#include "get_file_size.h"
#include "bit_operations.h"

#include <stdio.h>

status_code count_lsbi_changes_embedding(FILE* p_file, FILE* out_file, LSBI_count count[4], uint32_t in_file_size) {
    status_code exit_code = SUCCESS;
    rewind(p_file);
    rewind(out_file);

    for (int i = 0; i < 4; i++) {
        count[i].changed = 0;
        count[i].total = 0;
        count[i].should_invert = false;
    }

    // Skip headers in P_FILE and OUT_FILE
    if ((exit_code = skip_bmp_header(p_file)) != SUCCESS || (exit_code = skip_bmp_header(out_file)) != SUCCESS) {
        goto finally;
    }

    // We skip LSBI pattern in P_FILE and OUT_FILE
    if (fseek(p_file, 4, SEEK_CUR) || fseek(out_file, 4, SEEK_CUR) ) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    uint8_t p_byte, out_byte;
    // BMP files save data in BGR format, and in LSBI we need to skip the red byte
    color current_rgb_color = GREEN;
    while (fread(&p_byte, 1, 1, p_file) == 1 && in_file_size > 0) {
        if (ferror(p_file)) {
            exit_code = FILE_READ_ERROR;
            goto finally;
        }

        if (fread(&out_byte, 1, 1, out_file) != 1) {
            exit_code = FILE_READ_ERROR;
            goto finally;
        }
        in_file_size--; // Make sure we account for file size after skipping red bytes

        // Skip bytes used for red pixels
        color past_color = current_rgb_color;
        current_rgb_color = get_next_color(current_rgb_color);
        if (past_color == RED) {
            continue;
        }

        uint8_t pattern;
        if ((exit_code = get_LSBI_pattern(out_file, &pattern)) != SUCCESS) {
            goto finally;
        }

        count[pattern].total++;
        if ((p_byte & 0x01) != (out_byte & 0x01)) {
            // Check if the last bit in p_byte and out_byte are different
            count[pattern].changed++;
        }
    }

    // Check for errors in file read
    if (ferror(p_file)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }
    if (ferror(out_file)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    // Set should_invert to true when necessary
    for (int i = 0; i < 4; i++) {
        int not_changed = count[i].total - count[i].changed;
        if (count[i].changed >= not_changed)
            count[i].should_invert = true;
    }


finally:
    return exit_code;
}
