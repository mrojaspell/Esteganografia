#include "extract_lsbi_pattern_inversion.h"

#include "colors.h"

#include "bit_operations.h"
#include "get_file_size.h"

#include <stdint.h>
#include <stdbool.h>

status_code extract_lsbi_pattern_inversion(FILE * out_file, color* current_color){
    status_code exit_code = SUCCESS;

    // Start reading the file from the beginning
    rewind(out_file);

    // Skip BMP header in OUT_FILE
    if ((exit_code = skip_bmp_header(out_file)) != SUCCESS){
        goto finally;
    }

    // Check what LSBI patterns to invert
    bool patterns[4]; // TODO: make 4 a macro
    uint8_t read_byte;
    for (size_t i = 0; i < 4; i++) {
        bool bit;
        if ((exit_code = get_last_bit(out_file, &bit)) != SUCCESS){
            goto finally;
        }
        // Save if this pattern is to be inverted or not
        patterns[i] = bit;
        *current_color = get_next_color(*current_color);
    }

    // Iterate through the rest of the file, inverting the previously defined patterns
    while (fread(&read_byte, 1, 1, out_file) == 1) {
        if (ferror(out_file)) {
            exit_code = FILE_READ_ERROR;
            goto finally;
        }

        const color past_color = *current_color;
        *current_color = get_next_color(*current_color);
        if (past_color == RED) {
            continue;
        }

        // Invert the last bit if the pattern is marked to do so
        if ((exit_code = invert_last_bit_by_pattern(out_file, patterns)) != SUCCESS){
            goto finally;
        }

    }

    // Check for errors in file read
    if (!feof(out_file)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }
    
    finally:
    return exit_code;
}