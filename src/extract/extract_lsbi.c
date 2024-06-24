#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "print_error.h"
#include "get_file_size.h"
#include "extract_lsb1.h"
#include "extract_lsbi_pattern_inversion.h"
#include "colors.h"


// hidden as (real size (4 bytes total) || data || file extension (eg: .txt))
status_code extract_lsbi(const char* p_file_path, const char* out_file_path, encryption_alg encryption,
                         block_chaining_mode chaining, char* password) {
    status_code exit_code = SUCCESS;
    uint32_t secret_size = 0;
    color current_rgb_color = BLUE;
    FILE *p_file = NULL, *out_file = NULL, *tmp_file = NULL;
    uint8_t *out_buffer = NULL, *in_buffer = NULL, *decrypted_output = NULL;

    p_file = fopen(p_file_path, "r");
    if (p_file == NULL) {
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }
    out_file = fopen(out_file_path, "w+");
    if (out_file == NULL) {
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }
    tmp_file = tmpfile();
    if (tmp_file == NULL) {
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }

    if ((exit_code = copy_rest_of_file(p_file, tmp_file)) != SUCCESS) {
        goto finally;
    }

    if ((exit_code = extract_lsbi_pattern_inversion(tmp_file, &current_rgb_color)) != SUCCESS) {
        goto finally;
    }

    rewind(tmp_file);

    // Skip header
    if ((exit_code = skip_bmp_header(tmp_file)) != SUCCESS) {
        goto finally;
    }

    // Skip LSBI inversion pattern
    fseek(tmp_file, 4, SEEK_CUR);
    // We are now on the GREEN color, since we skipped the bmp header again
    current_rgb_color = GREEN;

    uint8_t size_buffer[SECRET_SIZE_IN_COVER_LSBI(FILE_LENGTH_BYTES)] = {0};
    if (fread(size_buffer, 1, SECRET_SIZE_IN_COVER_LSBI(FILE_LENGTH_BYTES), tmp_file) <
        SECRET_SIZE_IN_COVER_LSBI(FILE_LENGTH_BYTES)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    for (int i = 0; i < SECRET_SIZE_IN_COVER_LSBI(FILE_LENGTH_BYTES); i++) {
        const color past_color = current_rgb_color;
        current_rgb_color = get_next_color(current_rgb_color);
        if (past_color == RED) {
            continue;
        }
        secret_size = (secret_size << 1) | (size_buffer[i] & 0x1);
    }

    out_buffer = malloc(secret_size);
    size_t out_iterator = 0;
    if (out_buffer == NULL) {
        exit_code = MEMORY_ERROR;
        goto finally;
    }

    in_buffer = malloc(SECRET_SIZE_IN_COVER_LSBI(secret_size));
    size_t in_iterator = 0;
    if (in_buffer == NULL) {
        exit_code = MEMORY_ERROR;
        goto finally;
    }

    if (fread(in_buffer, 1, SECRET_SIZE_IN_COVER_LSBI(secret_size), tmp_file) < SECRET_SIZE_IN_COVER_LSBI(secret_size)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    for (; out_iterator < secret_size; out_iterator++) {
        uint8_t byte = 0;
        for (int limit = in_iterator + SECRET_SIZE_IN_COVER_LSBI(1); in_iterator < limit; in_iterator++) {
            const color past_color = current_rgb_color;
            current_rgb_color = get_next_color(current_rgb_color);
            if (past_color == RED) {
                continue;
            }
            byte = (byte << 1) | (in_buffer[in_iterator] & 0x1);
        }
        out_buffer[out_iterator] = byte;
    }

    password_metadata password_metadata = {0};

    if (password != 0) {
        uint32_t true_size = 0;
        password_metadata.password = password;
        exit_code = initialize_password_metadata(&password_metadata, encryption, chaining);
        if (exit_code != SUCCESS) {
            goto finally;
        }

        decrypted_output = malloc(secret_size);
        if (decrypted_output == NULL) {
            exit_code = MEMORY_ERROR;
            goto finally;
        }

        if (decrypt_payload(out_buffer, secret_size, decrypted_output, &password_metadata) == -1) {
            exit_code = ENCRYPTION_ERROR;
            goto finally;
        }
        true_size = *((uint32_t*)decrypted_output);
        const uint8_t* output_without_size = decrypted_output + 4;

        if (true_size > secret_size) {
            exit_code = ENCRYPTION_ERROR;
            goto finally;
        }

        if (fwrite(output_without_size, 1, true_size, out_file) < true_size) {
            exit_code = FILE_WRITE_ERROR;
        }
    } else {
        if (fwrite(out_buffer, 1, secret_size, out_file) < secret_size) {
            exit_code = FILE_WRITE_ERROR;
        }
    }


finally:
    if (p_file != NULL) {
        fclose(p_file);
    }
    if (out_file != NULL) {
        fclose(out_file);
    }
    if (tmp_file != NULL) {
        fclose(tmp_file);
    }
    if (out_buffer != NULL) {
        free(out_buffer);
    }
    if (in_buffer != NULL) {
        free(in_buffer);
    }
    if (decrypted_output != NULL) {
        free(decrypted_output);
    }
    return exit_code;
}
