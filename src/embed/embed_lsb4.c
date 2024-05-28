#include "embed_lsb4.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "get_file_size.h"


status_code embed_lsb4(char* in_file_path, char* p_file_path, char* out_file_path) {
    const off_t in_file_size = get_file_size(in_file_path);
    const off_t p_file_size = get_file_size(p_file_path);
    char extension[10] = {0};
    status_code exit_code = SUCCESS;

    sscanf(in_file_path, "%*[^.].%s", extension);
    const int extension_size = strlen(extension);


    if (in_file_size == -1 || p_file_size == -1) {
        exit_code = FILE_OPEN_ERROR;
        goto handle_errors;
    }

    // Since we are using lsb4, we use 4 bits of carrier per byte of secret, so the secret can be at most 1/2 of the carrier
    if (p_file_size < in_file_size * 2 + 4 + extension_size + BMP_HEADER_SIZE) {
        // TODO: Show maximum secret size for bmp
        exit_code = SECRET_TOO_BIG;
        goto handle_errors;
    }

    FILE* in_file = fopen(in_file_path, "r");
    if (in_file == NULL) {
        exit_code = FILE_OPEN_ERROR;
        goto handle_errors;
    }

    FILE* p_file = fopen(p_file_path, "r");
    if (p_file == NULL) {
        exit_code = FILE_OPEN_ERROR;
        goto handle_errors;
    }

    FILE* out_file = fopen(out_file_path, "w");
    if (out_file == NULL) {
        exit_code = FILE_OPEN_ERROR;
        goto handle_errors;
    }

    uint8_t in_buffer[BUFSIZ] = {0};
    uint8_t p_buffer[BUFSIZ] = {0};
    uint8_t out_buffer[BUFSIZ] = {0};

    fread(in_buffer, 1, BMP_HEADER_SIZE, p_file);
    if (fwrite(in_buffer, 1, BMP_HEADER_SIZE, out_file) == 0) {
        exit_code = FILE_WRITE_ERROR;
        goto handle_errors;
    }

    size_t bytes_read_in = 0, bytes_read_p = 0;

    while ((bytes_read_p = fread(p_buffer, 1, sizeof(p_buffer), in_file)) > 0) {
        bytes_read_in = fread(in_buffer, 1, bytes_read_p / 2, p_file);
        if (bytes_read_in == 0) {
            if (feof(p_file))
                break;

            exit_code = FILE_READ_ERROR;
            goto handle_errors;
        }

        for (size_t i = 0; i < bytes_read_in;) {
            const uint8_t byte = in_buffer[i / 2];
            const uint8_t high_nibble = byte >> 4;
            const uint8_t low_nibble = byte & 0x0F;
            out_buffer[i++] = (p_buffer[i] & 0xF0) | high_nibble;
            out_buffer[i++] = (p_buffer[i] & 0xF0) | low_nibble;
        }

        if (fwrite(out_buffer, 1, bytes_read_in, out_file) == 0) {
            exit_code = FILE_WRITE_ERROR;
            goto handle_errors;
        }
    }

    for (int i = 0; i < extension_size; i++) {
        const uint8_t byte = extension[i];
        const uint8_t high_nibble = byte >> 4;
        const uint8_t low_nibble = byte & 0x0F;
        out_buffer[i * 2] = (p_buffer[i] & 0xF0) | high_nibble;
        out_buffer[i * 2 + 1] = (p_buffer[i] & 0xF0) | low_nibble;
    }

    if (fwrite(out_buffer, 1, extension_size * 2, out_file) == 0) {
        exit_code = FILE_WRITE_ERROR;
        goto handle_errors;
    }

handle_errors:
    if (in_file != NULL)
        fclose(in_file);
    if (p_file != NULL)
        fclose(p_file);
    if (out_file != NULL)
        fclose(out_file);

    return exit_code;
}
