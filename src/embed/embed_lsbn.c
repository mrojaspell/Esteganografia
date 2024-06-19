#include "embed_lsbn.h"
#include "get_file_size.h"
#include "print_error.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

// Auxiliary functions
status_code copy_from_file_to_file(FILE* copy_from, FILE* copy_to, const unsigned int len);
status_code copy_rest_of_file(FILE* copy_from, FILE* copy_to);
status_code embed_bytes_lsbn(const unsigned int n, FILE* p_file, FILE* out_file, const uint8_t* bytes_to_embed,
                             unsigned int len);
status_code embed_file_lsbn(const unsigned int n, FILE* p_file, FILE* out_file, FILE* in_file);
status_code embed_number_lsbn(const unsigned int n, FILE* p_file, FILE* out_file, unsigned int number_to_embed);


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
    if ((exit_code = copy_from_file_to_file(p_file, out_file, BMP_HEADER_SIZE)) != SUCCESS) {
        goto finally;
    }

    // Embed in_file filesize in out_file
    if ((exit_code = embed_number_lsbn(n, p_file, out_file, in_file_size)) != SUCCESS) {
        goto finally;
    }

    // Embed in_file content in out_file
    if ((exit_code = embed_file_lsbn(n, p_file, out_file, in_file)) != SUCCESS) {
        goto finally;
    }

    // Embed in_file extension in out_file
    if ((exit_code = embed_bytes_lsbn(n, p_file, out_file, (uint8_t*)extension, in_extension_size + 1)) != SUCCESS) {
        goto finally;
    }

    // Copy the rest of p_file to out_file
    if ((exit_code = copy_rest_of_file(p_file, out_file)) != SUCCESS) {
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

// Embed len bytes from in_file in p_file, and save it in out_file
status_code embed_bytes_lsbn(const unsigned int n, FILE* p_file, FILE* out_file, const uint8_t* bytes_to_embed,
                             unsigned int len) {
    status_code exit_code = SUCCESS;

    // Calculate bytes needed from p_file to embed len bytes
    unsigned int bytes_needed = BYTES_TO_EMBED_BYTE(n) * len;

    // Copy the needed bytes from p_file to buffer
    uint8_t* buffer = malloc(bytes_needed);
    if (buffer == NULL) {
        exit_code = MEMORY_ERROR;
        goto finally;
    }
    if (fread(buffer, 1, bytes_needed, p_file) < bytes_needed) {
        exit_code = FILE_READ_ERROR;
        goto finally;
    }

    // Calculate the masks to do the embed
    const uint8_t P_MASK = 0xFF + 1 - pow(2, n);
    const uint8_t EMBED_MASK = 0xFF - P_MASK;


    // Embed bytes to buffer
    unsigned int it_buffer = 0;
    for (int i = 0; i < len; i++) {
        uint8_t byte_to_embed = bytes_to_embed[i];
        for (int j = 0; j < BYTES_TO_EMBED_BYTE(n); j += 1) {
            const uint8_t offset = 8 - ((j + 1) * n);
            buffer[it_buffer] = ((buffer[it_buffer] & P_MASK) | ((byte_to_embed >> offset) & EMBED_MASK));
            it_buffer++;
        }
    }


    // Save buffer to OUT file
    if (fwrite(buffer, 1, bytes_needed, out_file) < bytes_needed) {
        exit_code = FILE_WRITE_ERROR;
        goto finally;
    }


finally:
    if (buffer != NULL)
        free(buffer);
    return exit_code;
}

status_code embed_file_lsbn(const unsigned int n, FILE* p_file, FILE* out_file, FILE* in_file) {
    status_code exit_code = SUCCESS;

    uint8_t buffer[BUFSIZ];
    unsigned int read_bytes;
    while ((read_bytes = fread(buffer, 1, BUFSIZ, in_file)) > 0) {
        // Error while reading the file
        if (ferror(in_file))
            return FILE_READ_ERROR;

        exit_code = embed_bytes_lsbn(n, p_file, out_file, buffer, read_bytes);
        if (exit_code != SUCCESS) {
            return exit_code;
        }
    }

    // If we couldn't finish reading the file
    if (!feof(in_file))
        return FILE_READ_ERROR;

    return exit_code;
}

status_code embed_number_lsbn(const unsigned int n, FILE* p_file, FILE* out_file, unsigned int number_to_embed) {
    const uint8_t* original = (uint8_t*)&number_to_embed;
    // Because we're making an array out of an 32bit number, endianess is kicking our ass
    // So we decided to reverse the array manually, knowing that it's always size 4;
    uint8_t array[4];
    array[0] = original[3];
    array[1] = original[2];
    array[2] = original[1];
    array[3] = original[0];
    return embed_bytes_lsbn(n, p_file, out_file, (uint8_t*)array, 4);
}
