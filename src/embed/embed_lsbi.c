#include "embed_lsbi.h"
#include "embed_lsbn.h"
#include "embed_utils.h"
#include "get_file_size.h"
#include "print_error.h"
#include "colors.h"
#include "lsbi_invert_patterns.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>


status_code embed_lsbi(unsigned char n, char* in_file_path, char* p_file_path, char* out_file_path,
                       encryption_alg encryption, block_chaining_mode chaining, char* password) {
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
    out_file = fopen(out_file_path, "w+");
    if (out_file == NULL) {
        print_error("Error al abrir el archivo de salida\n");
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }

    // Retrieve BMP header size for P_FILE
    uint32_t P_BMP_HEADER_SIZE;
    if ((exit_code = get_bmp_header_size(p_file, &P_BMP_HEADER_SIZE)) != SUCCESS) {
        goto finally;
    }

    // Check if P file is big enough to embed IN file
    // (add 1 to account for \0 in extension)
    const int IN_PAYLOAD_SIZE = FILE_LENGTH_BYTES + in_file_size + in_extension_size + 1;
    const int P_AVAILABLE_SIZE = (p_file_size - P_BMP_HEADER_SIZE) / BYTES_TO_EMBED_BYTE_LSBI(n);
    if (P_AVAILABLE_SIZE < IN_PAYLOAD_SIZE) {
        // TODO: Show maximum secret size for bmp
        print_error("El archivo bmp no puede albergar el archivo a ocultar completo\n");
        exit_code = SECRET_TOO_BIG;
        goto finally;
    }

    // Copy header from P file to OUT file
    if ((exit_code = copy_from_file_to_file(p_file, out_file, P_BMP_HEADER_SIZE)) != SUCCESS) {
        goto finally;
    }

    // Embed 4 zeros using LSB1, to save space to store if the patterns 00, 01, 10, 11 are inverted or not
    // After embedding, we have to see if it is necessary to change these bytes or not
    if ((exit_code = embed_number_lsbni(1, p_file, out_file, 0, false, NULL)) != SUCCESS) {
        goto finally;
    }

    // Set the first color to green (because embed_number_lsbni already used BLUE-GREEN-RED-BLUE)
    color current_color = GREEN;

    // Embed in_file filesize in out_file using LSBI
    if ((exit_code = embed_number_lsbni(n, p_file, out_file, in_file_size, true, &current_color)) != SUCCESS) {
        goto finally;
    }

    // Embed in_file content in out_file
    if ((exit_code = embed_file_lsbni(n, p_file, out_file, in_file, true, &current_color)) != SUCCESS) {
        goto finally;
    }

    // Embed in_file extension in out_file
    if ((exit_code = embed_bytes_lsbni(n, p_file, out_file, (uint8_t*)extension, in_extension_size + 1, true,
                                       &current_color)) != SUCCESS) {
        goto finally;
    }

    // Copy the rest of p_file to out_file
    if ((exit_code = copy_rest_of_file(p_file, out_file)) != SUCCESS) {
        goto finally;
    }

    // Apply the bit inversion algorithm
    if ((exit_code = lsbi_invert_patterns(p_file, out_file, in_file_size)) != SUCCESS) {
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
