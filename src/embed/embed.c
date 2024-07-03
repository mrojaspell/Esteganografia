#include <string.h>
#include <stdio.h>

#include "embed.h"

#include <ssl_helpers.h>
#include <stdlib.h>

#include "embed_utils.h"
#include "get_file_size.h"
#include "lsbi_invert_patterns.h"
#include "enums.h"
#include "print_error.h"
#include "status_codes.h"

#define REQUIRED_PARAMS_NO 4
#define REQUIRED_ARGS_NO REQUIRED_PARAMS_NO * 2

#define FCLOSE(file_ptr)        \
    do {                        \
        if (file_ptr != NULL) { \
            fclose(file_ptr);   \
            file_ptr = NULL;    \
        }                       \
    } while (0)

#define FREE(alloc_ptr)             \
    do {                            \
        if (alloc_ptr != NULL) {    \
            free(alloc_ptr);        \
            alloc_ptr = NULL;       \
        }                           \
    } while (0)

struct params {
    // Required
    char* in_file;
    char* p_bitmap_file;
    char* out_bitmap_file;
    steg_alg steg;
    // Optional
    encryption_alg encryption; // default = AES128
    block_chaining_mode chaining; // default = CBC
    char* password;
};

status_code set_params_embed(struct params* params, int argc, char* argv[]);
status_code set_param_embed(struct params* params, char* arg_name, char* arg_value);

static status_code open_files(struct params* params, FILE** p_file, FILE** in_file, FILE** out_file) {
    *p_file = fopen(params->p_bitmap_file, "r");
    if (*p_file == NULL) {
        print_error("Couldn't open p file\n");
        return FILE_OPEN_ERROR;
    }
    *in_file = fopen(params->in_file, "r");
    if (*in_file == NULL) {
        print_error("Couldn't open in file\n");
        FCLOSE(*in_file);
        return FILE_OPEN_ERROR;
    }

    *out_file = fopen(params->out_bitmap_file, (params->steg == LSBI) ? "w+" : "w");
    if (*out_file == NULL) {
        print_error("Coudn't open out file\n");
        FCLOSE(*p_file);
        FCLOSE(*in_file);
        return FILE_OPEN_ERROR;
    }

    return SUCCESS;
}

static status_code prepare_payload(uint8_t** payload, uint32_t* payload_size, off_t in_file_size, FILE* in_file,
                                   char* extension,
                                   const int in_extension_size) {
    // Allocate enough memory for in_file_size (4 bytes) + in_file + extension (plus \0)
    *payload_size = sizeof(uint32_t) + in_file_size + in_extension_size + 1;
    *payload = malloc(*payload_size);
    if (*payload == NULL) {
        return MEMORY_ERROR;
    }

    // Write the file size
    uint8_t file_size_buffer[4];
    file_size_buffer[0] = (in_file_size >> 24) & 0xFF;
    file_size_buffer[1] = (in_file_size >> 16) & 0xFF;
    file_size_buffer[2] = (in_file_size >> 8) & 0xFF;
    file_size_buffer[3] = in_file_size & 0xFF;

    memcpy(*payload, file_size_buffer, sizeof(uint32_t));

    // Write the original file
    if (fread(*payload + sizeof(uint32_t), 1, in_file_size, in_file) < in_file_size) {
        FREE(*payload);
        return FILE_READ_ERROR;
    }

    // Write the extension
    memcpy(*payload + sizeof(uint32_t) + in_file_size, extension, in_extension_size + 1);

    return SUCCESS;
}

static status_code check_lsbn_file_sizes(int n, uint32_t payload_size, off_t p_file_size, uint32_t p_bmp_header_size) {
    const int p_available_size = (p_file_size - p_bmp_header_size) / BYTES_TO_EMBED_BYTE_LSBN(n);
    if (p_available_size < payload_size) {
        print_error("BMP file can't fit whole secret file, it can fit at max %d bytes\n", p_available_size);
        return SECRET_TOO_BIG;
    }
    return SUCCESS;
}

static status_code check_lsbi_file_sizes(uint32_t payload_size, off_t p_file_size, uint32_t p_bmp_heder_size) {
    const int p_available_size = (p_file_size - p_bmp_heder_size) / BYTES_TO_EMBED_BYTE_LSBI(1);
    if (p_available_size < payload_size) {
        print_error("BMP file can't fit whole secret file, it can fit at max %d bytes\n", p_available_size);
        return SECRET_TOO_BIG;
    }
    return SUCCESS;
}

status_code embed(int argc, char* argv[]) {
    // Initialize params
    struct params params = {0};
    FILE *in_file = NULL, *p_file = NULL, *out_file = NULL;
    uint8_t *encrypted_output = NULL, *plaintext_input = NULL;
    uint32_t encrypted_size = 0;
    password_metadata password_metadata = {0};
    char extension[MAX_EXTENSION_SIZE];

    // Set params
    status_code status = set_params_embed(&params, argc, argv);
    if (status != SUCCESS) {
        print_error("Error in -embed params\n");
        return status;
    }

    const int in_extension_size = get_file_extension(params.in_file, extension);

    off_t in_file_size = get_file_size(params.in_file);
    off_t p_file_size = get_file_size(params.p_bitmap_file);

    if (in_file_size > UINT32_MAX) {
        status = SECRET_TOO_BIG;
        goto finally;
    }

    if (in_file_size == -1 || p_file_size == -1) {
        status = FILE_OPEN_ERROR;
        goto finally;
    }

    if (params.password != NULL) {
        password_metadata.password = params.password;

        if ((status = initialize_password_metadata(&password_metadata, params.encryption, params.chaining)) !=
            SUCCESS) {
            print_error("Could not initialize password metadata");
            goto finally;
        }
    }

    if ((status = open_files(&params, &p_file, &in_file, &out_file)) != SUCCESS) {
        goto finally;
    }

    uint32_t p_bmp_header_size = 0;
    if ((status = copy_bmp_header_file_to_file(p_file, out_file, &p_bmp_header_size)) != SUCCESS) {
        goto finally;
    }

    // The first byte after the header is the one for the BLUE channel.
    color current_color = BLUE;

    uint8_t* payload = NULL;
    uint32_t payload_size = 0;

    if ((status = prepare_payload(&plaintext_input, &payload_size, in_file_size, in_file, extension, in_extension_size))
        != SUCCESS) {
        goto finally;
    }

    payload = plaintext_input;

    // Encrypt payload
    if (params.password != NULL) {
        if ((status = encrypt_payload(plaintext_input, in_file_size + sizeof(uint32_t) + in_extension_size + 1,
                                      &encrypted_output, &encrypted_size, &password_metadata)) != SUCCESS) {
            goto finally;
        }
        payload = encrypted_output;
        payload_size = encrypted_size;
    }


    switch (params.steg) {
    case LSB1:
        status = check_lsbn_file_sizes(1, payload_size, p_file_size, p_bmp_header_size);
        if (status != SUCCESS) goto finally;

        status = embed_bytes_lsbni(1, p_file, out_file, payload, payload_size, false, NULL);
        if (status != SUCCESS) goto finally;

        status = copy_rest_of_file(p_file, out_file);
        break;
    case LSB4:
        status = check_lsbn_file_sizes(4, payload_size, p_file_size, p_bmp_header_size);
        if (status != SUCCESS) goto finally;

        status = embed_bytes_lsbni(4, p_file, out_file, payload, payload_size, false, NULL);
        if (status != SUCCESS) goto finally;

        status = copy_rest_of_file(p_file, out_file);
        break;
    case LSBI:
        status = check_lsbi_file_sizes(payload_size, p_file_size, p_bmp_header_size);
        if (status != SUCCESS) goto finally;

        // Save space for bit inversion pattern
        status = copy_from_file_to_file(p_file, out_file, 4);
        if (status != SUCCESS) goto finally;
        // After bit pattern, we're in green color.
        current_color = GREEN;

        status = embed_bytes_lsbni(1, p_file, out_file, payload, payload_size, true, &current_color);
        if (status != SUCCESS) goto finally;

        status = copy_rest_of_file(p_file, out_file);
        if (status != SUCCESS) goto finally;

        status = lsbi_invert_patterns(p_file, out_file, payload_size);
        break;
    default:
        return INVALID_STEG_ALG;
    }

finally:
    if (in_file != NULL) {
        FCLOSE(in_file);
    }
    if (out_file != NULL) {
        FCLOSE(out_file);
    }
    if (p_file != NULL) {
        FCLOSE(p_file);
    }
    if (encrypted_output != NULL) {
        FREE(encrypted_output);
    }
    if (plaintext_input != NULL) {
        FREE(plaintext_input);
    }

    return status;
}

status_code set_params_embed(struct params* params, int argc, char* argv[]) {
    // Check correct number
    if (argc < REQUIRED_ARGS_NO || argc % 2 == 1)
        return ILLEGAL_ARGUMENTS;

    // Set params struct
    for (int i = 0; i < argc; i += 2) {
        int status = set_param_embed(params, argv[i], argv[i + 1]);
        if (status != SUCCESS)
            return status;
    }

    // Check that the struct is valid
    if (params->in_file == NULL || params->p_bitmap_file == NULL || params->out_bitmap_file == NULL || params->steg ==
        UNSPECIFIED_STEG)
        return ILLEGAL_ARGUMENTS;

    return SUCCESS;
}

status_code set_param_embed(struct params* params, char* arg_name, char* arg_value) {
    bool error_ocurred = false;
    if (!strcmp(arg_name, "-in")) {
        params->in_file = arg_value;
    } else if (!strcmp(arg_name, "-p")) {
        params->p_bitmap_file = arg_value;
    } else if (!strcmp(arg_name, "-out")) {
        params->out_bitmap_file = arg_value;
    } else if (!strcmp(arg_name, "-steg")) {
        params->steg = get_steg_alg(arg_value);
        error_ocurred = (params->steg == UNSPECIFIED_STEG);
    } else if (!strcmp(arg_name, "-a")) {
        params->encryption = get_encryption_alg(arg_value);
        //error_ocurred = (params->encryption == UNSPECIFIED_ENC);
        // la linea de arriba esta mal?
    } else if (!strcmp(arg_name, "-m")) {
        params->chaining = get_chaining_mode(arg_value);
        //error_ocurred = (params->chaining == UNSPECIFIED_CHAIN);
        // la linea de arriba esta mal?
    } else if (!strcmp(arg_name, "-pass")) {
        params->password = arg_value;
    } else {
        error_ocurred = true;
    }

    if (error_ocurred)
        return ILLEGAL_ARGUMENTS;
    return SUCCESS;
}
