#include "embed_lsbn.h"
#include "get_file_size.h"
#include "print_error.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>


status_code embed_lsbn(unsigned char n, char* in_file_path, char* p_file_path, char* out_file_path) {
    status_code exit_code = SUCCESS;
    FILE *in_file = NULL, *p_file = NULL, *out_file = NULL;
    uint8_t* file_size_buffer = NULL;

    // Get IN file extension
    char extension[MAX_EXTENSION_SIZE] = {0};
    const int in_extension_size = get_file_extension(in_file_path, extension);
    // Get IN and P file sizes
    off_t in_file_size = get_file_size(in_file_path);
    off_t p_file_size = get_file_size(p_file_path);

    // Check for errors
    if (in_extension_size == 0 || in_file_size == -1 || p_file_size == -1) {
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }


    // (add 1 to account for \0 in extension)
    const int IN_PAYLOAD_SIZE = FILE_LENGTH_BYTES + BMP_HEADER_SIZE + in_file_size + in_extension_size + 1;

    // Check if P file is big enough to embed IN file
    if (p_file_size / BYTES_TO_EMBED_BYTE(n) < IN_PAYLOAD_SIZE) {
        // TODO: Show maximum secret size for bmp
        print_error("El archivo bmp no puede albergar el archivo a ocultar completo\n");
        exit_code = SECRET_TOO_BIG;
        goto finally;
    }


    // Open IN file, read mode
    in_file = fopen(in_file_path, "r");
    if (in_file == NULL) {
        print_error("Error al abrir el archivo a ocultar\n");
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }
    // Open P file, read mode
    p_file = fopen(p_file_path, "r");
    if (p_file == NULL) {
        print_error("Error al abrir el archivo a bmp\n");
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }
    // Open OUT file, write mode
    out_file = fopen(out_file_path, "w"); // todo check what happens if file already exists
    if (out_file == NULL) {
        print_error("Error al abrir el archivo de salida\n");
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }

    // Copy header from P file to OUT file
    unsigned char header[BMP_HEADER_SIZE];
    if (fread(header, 1, BMP_HEADER_SIZE, p_file) < BMP_HEADER_SIZE) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }
    if (fwrite(header, 1, BMP_HEADER_SIZE, out_file) < BMP_HEADER_SIZE) {
        exit_code = FILE_WRITE_ERROR;
        goto finally;
    };


    int BYTES_TO_EMBED_SIZE = SECRET_SIZE_IN_COVER_LSBN(n);
    // Get first P file bytes to embed size
    file_size_buffer = calloc(BYTES_TO_EMBED_SIZE, sizeof(u_int8_t));
    if (fread(file_size_buffer, 1, BYTES_TO_EMBED_SIZE, p_file) < BYTES_TO_EMBED_SIZE) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    const uint8_t P_MASK = 0xFF + 1 - pow(2, n);
    const uint8_t EMBED_MASK = 0xFF - P_MASK;
    // Embed size to P bytes and save in buffer
    for (int i = 0; i < BYTES_TO_EMBED_SIZE; i++) {
        const uint8_t offset = FILE_LENGTH_BITS - ((i + 1) * n);
        file_size_buffer[i] = (file_size_buffer[i] & P_MASK) | ((in_file_size >> offset) & EMBED_MASK);
    }

    // Save buffer to OUT file
    if (fwrite(file_size_buffer, 1, BYTES_TO_EMBED_SIZE, out_file) < BYTES_TO_EMBED_SIZE) {
        exit_code = FILE_WRITE_ERROR;
        goto finally;
    }


    // copiar payload del bmp modificando el lsb con el in al archivo a ocultar
    // cada bit de in va al lsb de un byte del bmp
    // TODO: Optimize this, instead of reading byte by byte, use a buffer
    uint8_t in_byte;
    uint8_t p_byte;
    uint8_t out_byte;

    while (fread(&in_byte, 1, 1, in_file) == 1) {
        for (int i = 8; i > 0; i -= n) {
            if (fread(&p_byte, 1, 1, p_file) < 1) {
                exit_code = FILE_READ_ERROR;
                goto finally;
            }
            out_byte = (p_byte & P_MASK) | ((in_byte >> (i - n)) & EMBED_MASK);
            if (fwrite(&out_byte, 1, 1, out_file) < 1) {
                exit_code = FILE_WRITE_ERROR;
                goto finally;
            }
        }
    }

    if (ferror(in_file)) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    // We use <= to also copy the '\0'
    for (int i = 0; i <= in_extension_size; i++) {
        for (int j = 8; j > 0; j -= n) {
            if (fread(&p_byte, 1, 1, p_file) < 1) {
                exit_code = FILE_READ_ERROR;
                goto finally;
            }
            out_byte = (p_byte & P_MASK) | ((extension[i] >> (j - n)) & EMBED_MASK);
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
    // Close resources
    if (in_file != NULL) {
        fclose(in_file);
    }
    if (p_file != NULL) {
        fclose(p_file);
    }
    if (out_file != NULL) {
        fclose(out_file);
    }
    if (file_size_buffer != NULL) {
        free(file_size_buffer);
    }

    return exit_code;
}
