#include "../include/extract_lsb4.h"
#include "../include/print_error.h"
#include "../include/get_file_size.h"
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

// hidden as (real size (4 bytes total) || data || file extension (eg: .txt\0))
status_code extract_lsb4(const char* p_bmp, const char* out_file_path) {
    status_code exit_code = SUCCESS;
    uint32_t size = 0;
    FILE *p_file = NULL, *out_file = NULL;
    uint8_t *out_buffer = NULL, *in_buffer = NULL;

    p_file = fopen(p_bmp, "r");
    if (p_file == NULL) {
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }
    out_file = fopen(out_file_path, "w");
    if (out_file == NULL) {
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }

    // Skip header
    if (fseek(p_file, BMP_HEADER_SIZE, SEEK_SET)) {
        print_error(strerror(errno));
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    // Get the hidden file size
    uint8_t size_buffer[SECRET_SIZE_IN_COVER_LSB4(SECRET_SIZE_BYTES)] = {0};
    if (fread(size_buffer, 1, SECRET_SIZE_IN_COVER_LSB4(SECRET_SIZE_BYTES), p_file) <
        SECRET_SIZE_IN_COVER_LSB4(SECRET_SIZE_BYTES)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    for (int i = 0; i < SECRET_SIZE_IN_COVER_LSB4(SECRET_SIZE_BYTES); i++) {
        size = (size << 4) | (size_buffer[i] & 0x0F);
    }


    out_buffer = malloc(size);
    size_t out_iterator = 0;
    if (out_buffer == NULL) {
        exit_code = MEMORY_ERROR;
        goto finally;
    }

    in_buffer = malloc(SECRET_SIZE_IN_COVER_LSB4(size));
    size_t in_iterator = 0;
    if (in_buffer == NULL) {
        exit_code = MEMORY_ERROR;
        goto finally;
    }

    if (fread(in_buffer, 1, SECRET_SIZE_IN_COVER_LSB4(size), p_file) < SECRET_SIZE_IN_COVER_LSB4(size)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    for (; out_iterator < size; out_iterator++) {
        uint8_t byte = 0;
        for (int limit = in_iterator + SECRET_SIZE_IN_COVER_LSB4(1); in_iterator < limit; in_iterator++) {
            byte = (byte << 4) | (in_buffer[in_iterator] & 0x0F);
        }
        out_buffer[out_iterator] = byte;
    }

    if (fwrite(out_buffer, 1, size, out_file) < size) {
        exit_code = FILE_WRITE_ERROR;
    }

finally:
    if (p_file != NULL)
        fclose(p_file);
    if (out_file != NULL)
        fclose(out_file);
    if (out_buffer != NULL)
        free(out_buffer);
    if (in_buffer != NULL)
        free(in_buffer);

    return exit_code;
}