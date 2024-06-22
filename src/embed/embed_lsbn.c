#include "embed_lsbn.h"
#include "embed_utils.h"
#include "get_file_size.h"
#include "print_error.h"
#include "ssl_helpers.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


status_code embed_lsbn(unsigned char n, char* in_file_path, char* p_file_path, char* out_file_path,
                       encryption_alg encryption, block_chaining_mode chaining, char* password) {
    status_code exit_code = SUCCESS;
    FILE *in_file = NULL, *p_file = NULL, *out_file = NULL;

    // Get IN file extension
    char extension[MAX_EXTENSION_SIZE] = {0};
    const int in_extension_size = get_file_extension(in_file_path, extension);
    // Get IN and P file sizes
    off_t in_file_size = get_file_size(in_file_path);
    off_t p_file_size = get_file_size(p_file_path);

    int encrypted_size = 0;
    uint8_t* encrypted_output = NULL;
    uint32_t encrypted_output_size = 0;
    uint8_t* plaintext_input = NULL;

    if (in_file_size > UINT32_MAX) {
        exit_code = SECRET_TOO_BIG;
        goto finally;
    }

    // Check for errors
    if (in_extension_size == 0 || in_file_size == -1 || p_file_size == -1) {
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }


    // Check if P file is big enough to embed IN file
    // (add 1 to account for \0 in extension)
    const int IN_PAYLOAD_SIZE = FILE_LENGTH_BYTES + in_file_size + in_extension_size + 1;
    const int P_AVAILABLE_SIZE = (p_file_size - BMP_HEADER_SIZE) / BYTES_TO_EMBED_BYTE_LSBN(n);
    if (P_AVAILABLE_SIZE < IN_PAYLOAD_SIZE) {
        // TODO: Show maximum secret size for bmp
        print_error("El archivo bmp no puede albergar el archivo a ocultar completo\n");
        exit_code = SECRET_TOO_BIG;
        goto finally;
    }

    password_metadata password_metadata = {0};
    if (password != 0) {
        // tenemos password para encriptar
        password_metadata.password = password;
        int status = initialize_password_metadata(&password_metadata, encryption, chaining);
        if (status != SUCCESS) {
            print_error("Could not initialize password metadata");
            exit_code = status;
            goto finally;
        }
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
    out_file = fopen(out_file_path, "w");
    if (out_file == NULL) {
        print_error("Error al abrir el archivo de salida\n");
        exit_code = FILE_OPEN_ERROR;
        goto finally;
    }

    // Copy header from P file to OUT file
    if ((exit_code = copy_bmp_header_file_to_file(p_file, out_file)) != SUCCESS) {
        goto finally;
    }



    if (password != NULL) {
        //asignar memorias
        plaintext_input = malloc(in_file_size + sizeof(in_file_size) + in_extension_size + 1);
        if (plaintext_input == NULL) {
            exit_code = MEMORY_ERROR;
            goto finally;
        }


        // escribir el tamaño en plaintext_input
        memcpy(plaintext_input, &in_file_size, sizeof(uint32_t));
        // escribir el archivo original (in_file) en plaintext_input
        if (fread(plaintext_input + sizeof(uint32_t), 1, in_file_size, in_file) < in_file_size) {
            exit_code = FILE_READ_ERROR;
            goto finally;
        }
        // escribir la extension en plaintext_input
        memcpy(plaintext_input + sizeof(uint32_t) + in_file_size, extension, in_extension_size + 1);

        // encriptar el contenido de plaintext_input en encrypted_output
        encrypted_size = encrypt_payload(plaintext_input, in_file_size + sizeof(uint32_t) + in_extension_size + 1,
                                         &encrypted_output, &encrypted_output_size, &password_metadata);

        // escribir el tamaño del cifrado en el archivo de salida
        if ((exit_code = embed_number_lsbni(n, p_file, out_file, encrypted_size, false, NULL)) != SUCCESS) {
            goto finally;
        }
        if ((exit_code = embed_bytes_lsbni(n, p_file, out_file, encrypted_output, encrypted_output_size, false, NULL)) != SUCCESS) {
            goto finally;
        }

        if ((exit_code = copy_rest_of_file(p_file, out_file)) != SUCCESS) {
            goto finally;
        }
    }
    else {
        // Embed in_file filesize in out_file
        if ((exit_code = embed_number_lsbni(n, p_file, out_file, in_file_size, false, NULL)) != SUCCESS) {
            goto finally;
        }

        // Embed in_file content in out_file
        if ((exit_code = embed_file_lsbni(n, p_file, out_file, in_file, false, NULL)) != SUCCESS) {
            goto finally;
        }

        // Embed in_file extension in out_file
        if ((exit_code = embed_bytes_lsbni(n, p_file, out_file, (uint8_t*)extension, in_extension_size + 1, false, NULL)) !=
            SUCCESS) {
            goto finally;
        }

        // Copy the rest of p_file to out_file
        if ((exit_code = copy_rest_of_file(p_file, out_file)) != SUCCESS) {
            goto finally;
        }
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
    if (encrypted_output != NULL) {
        free(encrypted_output);
    }
    if (plaintext_input != NULL) {
        free(plaintext_input);
    }

    return exit_code;
}
