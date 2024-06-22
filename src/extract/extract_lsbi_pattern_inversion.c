#include "extract_lsbi_pattern_inversion.h"
#include "bit_operations.h"

#include <stdint.h>
#include <stdbool.h>

status_code extract_lsbi_pattern_inversion(FILE * out_file){
    status_code exit_code = SUCCESS;


    // TODO: skip headers

    // Start reading the file from the beginning
    rewind(out_file);

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
    }

    // Iterate through the rest of the file, inverting the previously defined patterns
    while (fread(&read_byte, 1, 1, out_file) == 1) {
        if (ferror(out_file)) {
            exit_code = FILE_READ_ERROR;
            goto finally;
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