#include "bit_operations.h"
#include <stdint.h>

status_code get_last_bit(FILE * file, bool * bit){
    status_code exit_code = SUCCESS;

    // Read the byte
    uint8_t read_byte;
    if (fread(&read_byte, 1, 1, file) != 1){
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    // Obtain the bit
    *bit = (bool)(read_byte & 0x01);

    finally:
    return exit_code;
}


status_code set_last_bit(FILE * file, bool bit){
    status_code exit_code = SUCCESS;

    // Read the byte
    uint8_t read_byte;
    if ((fread(&read_byte, 1, 1, file) != 1) || ferror(file)){
        exit_code = FILE_READ_ERROR;
        goto finally;
    }
    // Produce the modified byte (bit is either 0x00 or 0x01)
    if (bit){
        read_byte = (bool)(read_byte | 0x01);
    } else {
        read_byte = (bool)(read_byte & 0xFE);
    }
    // Go back 1 byte
    fseek(file, -1, SEEK_CUR);
    // Write the byte
    if (fwrite(&read_byte, 1, 1, file) != 1){
        exit_code = FILE_WRITE_ERROR;
        goto finally;
    }

    finally:
    return exit_code;
}


status_code invert_last_bit(FILE * file){
    status_code exit_code = SUCCESS;

    bool bit;

    if ((exit_code = get_last_bit(file, &bit)) != SUCCESS){
        goto finally;
    }

    // Invert the bit
    bit = !bit;

    // Go back 1 byte
    fseek(file, -1, SEEK_CUR);

    if ((exit_code = set_last_bit(file, bit)) != SUCCESS){
        goto finally;
    }

    finally:
    return exit_code;
}

status_code get_LSBI_pattern(FILE * file, uint8_t * pattern){
    status_code exit_code = SUCCESS;

    // Go back 1 byte
    fseek(file, -1, SEEK_CUR);

    // Read the byte
    uint8_t read_byte;
    if ((fread(&read_byte, 1, 1, file) != 1) || ferror(file)){
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    *pattern = (read_byte >> 1) & 3; // PATTERN: 2nd and 3rd last bit of the byte (3 is 0b11)

    finally:
    return exit_code;
}

status_code invert_last_bit_by_pattern(FILE * file, bool patterns[4]){
    status_code exit_code = SUCCESS;

    uint8_t pattern;
    if ((exit_code = get_LSBI_pattern(file, &pattern)) != SUCCESS){
        goto finally;
    }

    // If pattern isn't marked for inversion, do nothing
    if (!patterns[pattern]){
        goto finally;
    }

    // If pattern is marked for inversion, invert the last read byte
    fseek(file, -1, SEEK_CUR);
    if ((exit_code = invert_last_bit(file)) != SUCCESS){
        goto finally;
    }

    finally:
    return exit_code;
}