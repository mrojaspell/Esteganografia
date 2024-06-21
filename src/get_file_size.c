#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#include "include/get_file_size.h"
#include "status_codes.h"
#include "include/print_error.h"


off_t get_file_size(const char* filepath) {
    struct stat st;

    // Get file information
    if (stat(filepath, &st) == 0) {
        return st.st_size;
    }

    print_error("Error getting file information");
    return -1;
}

int get_file_extension(const char* filepath, char* extension) {
    const char* dot = strrchr(filepath, '.');
    if (dot == NULL) {
        *extension = '\0';
        return 0;
    }
    int i = 0;
    for (; dot[i]; i++) {
        extension[i] = dot[i];
    }
    extension[i] = '\0';
    return i;
}

status_code copy_bmp_header_file_to_file(FILE* copy_from, FILE* copy_to) {
    uint8_t first[BMP_OFFSET_SIZE + BMP_OFFSET_POSITION];

    if (fread(first, 1, BMP_OFFSET_SIZE + BMP_OFFSET_POSITION, copy_from) < BMP_OFFSET_SIZE + BMP_OFFSET_POSITION) {
        return FILE_READ_ERROR;
    }

    uint32_t payload_offset = 0;

    for (int i = BMP_OFFSET_POSITION + BMP_OFFSET_SIZE - 1; i >= BMP_OFFSET_POSITION; i--) {
        payload_offset = (payload_offset << BITS_IN_BYTES) | (first[i] & 0xFF);
    }

    rewind(copy_from);

    return copy_from_file_to_file(copy_from, copy_to, payload_offset);
}

status_code skip_bmp_header(FILE* file) {
    uint8_t first[BMP_OFFSET_SIZE + BMP_OFFSET_POSITION];

    if (fread(first, 1, BMP_OFFSET_SIZE + BMP_OFFSET_POSITION, file) < BMP_OFFSET_SIZE + BMP_OFFSET_POSITION) {
        return FILE_READ_ERROR;
    }

    uint32_t payload_offset = 0;

    for (int i = BMP_OFFSET_POSITION + BMP_OFFSET_SIZE - 1; i >= BMP_OFFSET_POSITION; i--) {
        payload_offset = (payload_offset << BITS_IN_BYTES) | (first[i] & 0xFF);
    }

    if (fseek(file,  payload_offset, SEEK_SET) != 0) {
        return FILE_READ_ERROR;
    }

    return SUCCESS;
}

// Copy len bytes from "copy_from" to "copy_to"
status_code copy_from_file_to_file(FILE* copy_from, FILE* copy_to, unsigned int len) {
    status_code exit_code = SUCCESS;
    uint8_t* buffer = malloc(len * sizeof(uint8_t));

    if (fread(buffer, 1, len, copy_from) < len) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }
    if (fwrite(buffer, 1, len, copy_to) < len) {
        exit_code = FILE_WRITE_ERROR;
        goto finally;
    };

finally:
    if (buffer != NULL)
        free(buffer);
    return exit_code;
}

status_code copy_rest_of_file(FILE* copy_from, FILE* copy_to) {
    uint8_t buffer[BUFSIZ] = {0};
    size_t read = 0;

    while ((read = fread(buffer, 1, BUFSIZ, copy_from)) > 0) {
        if (fwrite(buffer, 1, read, copy_to) < read) {
            print_error(strerror(errno));
            return FILE_WRITE_ERROR;
        }
    }

    if (ferror(copy_from)) {
        return FILE_READ_ERROR;
    }
    return SUCCESS;
}
