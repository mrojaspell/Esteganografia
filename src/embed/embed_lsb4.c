#include "embed_lsb4.h"
#include "get_file_size.h"
#include "print_error.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE_BYTES_ENCODED SECRET_SIZE_IN_COVER_LSB4(SECRET_SIZE_BYTES)

status_code embed_lsb4(char* in_file_path, char* p_file_path, char* out_file_path) {
    status_code exit_code = SUCCESS;

    char extension[MAX_EXTENSION_SIZE] = {0};
    const int extension_size = get_file_extension(in_file_path, extension);

    const off_t in_file_size = get_file_size(in_file_path);
    const off_t p_file_size = get_file_size(p_file_path);
    if (in_file_size == -1 || p_file_size == -1) {
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }

    // Since we are using lsb4, we use 4 bits of carrier per byte of secret, so the secret can be at most 1/2 of the carrier
    if (in_file_size * 8/4 > (p_file_size - BMP_HEADER_SIZE - extension_size)) {
        // TODO: Show maximum secret size for bmp
        print_error("El archivo bmp no puede albergar el archivo a ocultar completo\n");
        exit_code = SECRET_TOO_BIG;
        goto finally;
    }

    FILE* in_file = fopen(in_file_path, "r");
    if (in_file == NULL) {
        print_error("Error al abrir el archivo a ocultar\n");
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }

    FILE* p_file = fopen(p_file_path, "r");
    if (p_file == NULL) {
        print_error("Error al abrir el archivo a bmp\n");
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }

    FILE* out_file = fopen(out_file_path, "w");
    if (out_file == NULL) {
        print_error("Error al abrir el archivo de salida\n");
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }

    unsigned char header[BMP_HEADER_SIZE];
    if (fread(header, 1, BMP_HEADER_SIZE, p_file) < BMP_HEADER_SIZE) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }
    if (fwrite(header, 1, BMP_HEADER_SIZE, out_file) < BMP_HEADER_SIZE) {
        exit_code = FILE_WRITE_ERROR;
        goto finally;
    }

    // Saving the secret file size
    uint8_t file_size_buffer[SIZE_BYTES_ENCODED] = {0};
    if (fread(file_size_buffer, 1, SIZE_BYTES_ENCODED, p_file) < SIZE_BYTES_ENCODED) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    for (int i = 0; i < SIZE_BYTES_ENCODED; i++) {
        file_size_buffer[i] = (file_size_buffer[i] & 0xF0) | ((in_file_size >> (32 - (i + 1) * 4)) & 0x0F);
    }

    if (fwrite(file_size_buffer, 1, SIZE_BYTES_ENCODED, out_file) < SIZE_BYTES_ENCODED) {
        exit_code = FILE_WRITE_ERROR;
        goto finally;
    }

    uint8_t in_byte;
    uint8_t p_byte;
    uint8_t out_byte;

    while (fread(&in_byte, 1, 1, in_file) == 1) {
        for (int i = 8; i > 0; i = i - 4) {
            if (fread(&p_byte, 1, 1, p_file) < 1) {
                exit_code = FILE_READ_ERROR;
                goto finally;
            }
            out_byte = (p_byte & 0xF0) | ((in_byte >> (i - 4)) & 0x0F);
            if (fwrite(&out_byte, 1, 1, out_file) < 1) {
                exit_code = FILE_WRITE_ERROR;
                goto finally;
            }
        }
    }

    // fread(in_file) failed for a reason other than EOF
    if (ferror(in_file)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    // We use <= to also copy the '\0'
    for (int i = 0; i <= extension_size; i++) {
        for (int j = 8; j > 0; j -= 4) {
            if (fread(&p_byte, 1, 1, p_file) < 1) {
                exit_code = FILE_READ_ERROR;
                goto finally;
            }
            out_byte = (p_byte & 0xF0) | ((extension[i] >> (j - 4)) & 0x0F);
            if (fwrite(&out_byte, 1, 1, out_file) < 1) {
                exit_code = FILE_WRITE_ERROR;
                goto finally;
            }
        }
    }

    uint8_t buffer[BUFSIZ] = {0};
    size_t read = 0;

    while ((read = fread(buffer, 1, BUFSIZ, p_file)) > 0) {
        if (fwrite(buffer, 1, read, out_file) < read) {
            print_error(strerror(errno));
            exit_code = FILE_WRITE_ERROR;
            goto finally;
        }
    }

    if (ferror(p_file)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }


finally:
    if (in_file != NULL)
        fclose(in_file);
    if (p_file != NULL)
        fclose(p_file);
    if (out_file != NULL)
        fclose(out_file);

    return exit_code;
}
