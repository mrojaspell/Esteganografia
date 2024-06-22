#include <stdio.h>

#include "lsbi_invert_patterns.h"
#include "lsbi_count.h"
#include "colors.h"
#include "embed_utils.h"
#include "bit_operations.h"

status_code lsbi_invert_patterns(FILE * p_file, FILE * out_file){
    status_code exit_code = SUCCESS;

    LSBI_count pattern_count[4];
    if ((exit_code = count_lsbi_changes_embedding(p_file, out_file, pattern_count)) != SUCCESS){
        goto finally;
    }

    rewind(out_file);

    // TODO: skip BMP header

    // Embed if we inverted the patterns or not
    for (int i = 0; i < 4; i++){
        if ((exit_code = set_last_bit(out_file, pattern_count[i].should_invert)) != SUCCESS){
            goto finally;
        }
    }
    

    uint8_t p_byte;
    // BMP files save data in BGR format, and in LSBI we need to skip the red byte
    color current_rgb_color = BLUE;
    while (fread(&p_byte, 1, 1, p_file) == 1) {
        if (ferror(p_file)) {
            exit_code = FILE_READ_ERROR;
            goto finally;
        }

        // Skip bytes used for red pixels
        color past_color = current_rgb_color;
        current_rgb_color = get_next_color(current_rgb_color);
        if (past_color == RED) {
            continue;
        }

        uint8_t pattern;
        if ((exit_code = get_LSBI_pattern(out_file, &pattern)) != SUCCESS){
            goto finally;
        }

        if (pattern_count[pattern].should_invert){
            // Go back 1 byte (to modify the bit we just read and not the next)
            fseek(out_file, -1, SEEK_CUR);
            // Invert the bit from the last read byte
            invert_last_bit(out_file);
        }
    }

    // Check for errors in file read
    if (!feof(p_file) || !feof(out_file)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }


    finally:
        return exit_code;
}