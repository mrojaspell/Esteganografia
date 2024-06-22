#include <stdio.h>

#include "lsbi_invert_patterns.h"
#include "lsbi_count.h"
#include "colors.h"
#include "embed_utils.h"
#include "bit_operations.h"
#include "get_file_size.h"

status_code lsbi_invert_patterns(FILE* p_file, FILE* out_file, uint32_t in_payload_size) {
    status_code exit_code = SUCCESS;
    uint64_t hidden_file_size = in_payload_size * BYTES_TO_EMBED_BYTE_LSBI(1);

    LSBI_count pattern_count[4];
    if ((exit_code = count_lsbi_changes_embedding(p_file, out_file, pattern_count, hidden_file_size)) != SUCCESS) {
        goto finally;
    }

    rewind(out_file);

    // Skip BMP header in OUT_FILE
    if ((exit_code = skip_bmp_header(out_file)) != SUCCESS) {
        goto finally;
    }

    // BMP files save data in BGR format, and in LSBI we need to skip the red byte
    color current_rgb_color = BLUE;

    // Embed if we inverted the patterns or not
    for (int i = 0; i < 4; i++) {
        if ((exit_code = set_last_bit(out_file, pattern_count[i].should_invert)) != SUCCESS) {
            goto finally;
        }
        current_rgb_color = get_next_color(current_rgb_color);
    }

    uint8_t out_byte;
    while (fread(&out_byte, 1, 1, out_file) == 1 && hidden_file_size > 0) {
        if (ferror(p_file)) {
            exit_code = FILE_READ_ERROR;
            goto finally;
        }

        hidden_file_size--;
        // Skip bytes used for red pixels
        const color past_color = current_rgb_color;
        current_rgb_color = get_next_color(current_rgb_color);
        if (past_color == RED) {
            continue;
        }

        uint8_t pattern;
        // if ((exit_code = get_LSBI_pattern(out_file, &pattern)) != SUCCESS){
        //     goto finally;
        // }
        pattern = (out_byte >> 1) & 3;

        if (pattern_count[pattern].should_invert) {
            // Go back 1 byte (to modify the bit we just read and not the next)
            fseek(out_file, -1, SEEK_CUR);
            // Invert the bit from the last read byte
            // invert_last_bit(out_file);
            const uint8_t last_bit = !(out_byte & 1);
            const uint8_t new_byte = (out_byte & 0xFE) | last_bit;
            if (fwrite(&new_byte, 1, 1, out_file) < 1) {
                exit_code = FILE_WRITE_ERROR;
                goto finally;
            }
        }
    }

    // Check for errors in file read
    if (ferror(p_file) || ferror(out_file)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }


finally:
    return exit_code;
}
