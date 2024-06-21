#include "extract_lsb1.h"

#include <errno.h>

#include "get_file_size.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "print_error.h"

// hidden as (real size (4 bytes total) || data || file extension (eg: .txt))
status_code extract_lsb1(const char* p_bmp, const char* out_file_path, encryption_alg encryption, block_chaining_mode chaining, char * password) {
    status_code exit_code = SUCCESS;
    uint32_t size = 0;
    FILE *p_file = NULL, *out_file = NULL;
    uint8_t *out_buffer = NULL, *in_buffer = NULL;

    p_file = fopen(p_bmp, "r");
    if (p_file == NULL) {
        exit_code = FILE_OPEN_ERROR;
        printf("error con bmp\n");
        goto finally;
    }
    out_file = fopen(out_file_path, "w");
    if (out_file == NULL) {
        exit_code = FILE_OPEN_ERROR;
        printf("error con out\n");
        goto finally;
    }

    // Skip header
    if ((exit_code = skip_bmp_header(p_file)) != SUCCESS) {
        goto finally;
    }

    uint8_t size_buffer[SECRET_SIZE_IN_COVER_LSB1(FILE_LENGTH_BYTES)] = {0};
    if (fread(size_buffer, 1, SECRET_SIZE_IN_COVER_LSB1(FILE_LENGTH_BYTES), p_file) <
        SECRET_SIZE_IN_COVER_LSB1(FILE_LENGTH_BYTES)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    for (int i = 0; i < SECRET_SIZE_IN_COVER_LSB1(FILE_LENGTH_BYTES); i++) {
        size = (size << 1) | (size_buffer[i] & 0x1);
    }

    out_buffer = malloc(size);
    size_t out_iterator = 0;
    if (out_buffer == NULL) {
        exit_code = MEMORY_ERROR;
        goto finally;
    }

    in_buffer = malloc(SECRET_SIZE_IN_COVER_LSB1(size));
    size_t in_iterator = 0;
    if (in_buffer == NULL) {
        exit_code = MEMORY_ERROR;
        goto finally;
    }

    if (fread(in_buffer, 1, SECRET_SIZE_IN_COVER_LSB1(size), p_file) < SECRET_SIZE_IN_COVER_LSB1(size)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    for (; out_iterator < size; out_iterator++) {
        uint8_t byte = 0;
        for (int limit = in_iterator + SECRET_SIZE_IN_COVER_LSB1(1); in_iterator < limit; in_iterator++) {
            byte = (byte << 1) | (in_buffer[in_iterator] & 0x1);
        }
        out_buffer[out_iterator] = byte;
    }

    password_metadata password_metadata = {0};
    uint32_t true_size = size;

    if(password != 0) {
        password_metadata.password = password;
        exit_code = initialize_password_metadata(&password_metadata, encryption, chaining);
        if (exit_code != SUCCESS) {
            goto finally;
        }

        uint8_t * decrypted_output = malloc(size);
        if (decrypt_payload(out_buffer, size, decrypted_output, &password_metadata) != SUCCESS) {
            exit_code = 1;
            goto finally;
        }
        true_size = *((uint32_t *) decrypted_output);
        decrypted_output = decrypted_output+4;
    }

    if (fwrite(out_buffer, 1, true_size, out_file) < size) {
        exit_code = FILE_WRITE_ERROR;
    }


finally:
    if (p_file != NULL) {
        fclose(p_file);
    }
    if (out_file != NULL) {
        fclose(out_file);
    }
    if (out_buffer != NULL) {
        free(out_buffer);
    }
    if (in_buffer != NULL) {
        free(in_buffer);
    }
    return exit_code;
}
