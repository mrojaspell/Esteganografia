#include "embed_utils.h"
#include "print_error.h"
#include "get_file_size.h"

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

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

// Copy the rest of copy_from to copy_to, until EOF is reached
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

status_code embed_file_lsbni(const unsigned int n, FILE* p_file, FILE* out_file, FILE* in_file, bool lsbi, color * current_color) {
    status_code exit_code = SUCCESS;

    uint8_t buffer[BUFSIZ];
    unsigned int read_bytes;
    while ((read_bytes = fread(buffer, 1, BUFSIZ, in_file)) > 0) {
        // Error while reading the file
        if (ferror(in_file))
            return FILE_READ_ERROR;

        exit_code = embed_bytes_lsbni(n, p_file, out_file, buffer, read_bytes, lsbi, current_color);
        
        if (exit_code != SUCCESS) {
            return exit_code;
        }
    }

    // If we couldn't finish reading the file
    if (!feof(in_file))
        return FILE_READ_ERROR;

    return exit_code;
}

status_code embed_number_lsbni(const unsigned int n, FILE* p_file, FILE* out_file, uint32_t number_to_embed, bool lsbi, color * current_color) {
    const uint8_t* original = (uint8_t*)&number_to_embed;
    // Because we're making an array out of an 32bit number, endianess is kicking our ass
    // So we decided to reverse the array manually, knowing that it's always size 4;
    uint8_t array[4];
    array[0] = original[3];
    array[1] = original[2];
    array[2] = original[1];
    array[3] = original[0];

    return embed_bytes_lsbni(n, p_file, out_file, (uint8_t*)array, 4, lsbi, current_color);
}


// Embed len bytes from in_file in p_file, and save it in out_file
status_code embed_bytes_lsbni(const unsigned int n, FILE* p_file, FILE* out_file, const uint8_t* bytes_to_embed, unsigned int len, bool lsbi, color * current_color) {
    status_code exit_code = SUCCESS;

    // Calculate bytes needed from p_file to embed len bytes
    const unsigned int bytes_to_embed_byte = lsbi ? BYTES_TO_EMBED_BYTE_LSBI(n) : BYTES_TO_EMBED_BYTE_LSBN(n);
    const unsigned int bytes_needed = bytes_to_embed_byte * len;
    

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
        for (int j = 0; j < bytes_to_embed_byte; j += 1) {
            // In LSBI we don't alter color RED
            if (lsbi) {
                color past_color = *current_color;
                *current_color = get_next_color(past_color);
                if ((past_color == RED)){
                    continue;
                }
            }
            
            // Embed the byte
            const uint8_t offset = 8 - ((j + 1) * n);
            buffer[it_buffer] = ((buffer[it_buffer] & P_MASK) | ((byte_to_embed >> offset) & EMBED_MASK));

            // Prepare for next loop
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