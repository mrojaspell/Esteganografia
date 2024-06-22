#include "embed_utils.h"
#include "print_error.h"
#include "get_file_size.h"

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>


status_code embed_file_lsbni(const unsigned int n, FILE* p_file, FILE* out_file, FILE* in_file, bool lsbi,
                             color* current_color) {
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

status_code embed_number_lsbni(const unsigned int n, FILE* p_file, FILE* out_file, uint32_t number_to_embed, bool lsbi,
                               color* current_color) {
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
status_code embed_bytes_lsbni(const unsigned int n, FILE* p_file, FILE* out_file, const uint8_t* bytes_to_embed,
                              unsigned int len, bool lsbi, color* current_color) {
    status_code exit_code = SUCCESS;

    // Calculate bytes needed from p_file to embed len bytes
    const unsigned int bytes_to_embed_byte = BYTES_TO_EMBED_BYTE_LSBN(n);
    const unsigned int bytes_needed = (lsbi ? BYTES_TO_EMBED_BYTE_LSBI(n) : BYTES_TO_EMBED_BYTE_LSBN(n)) * len;


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
        const uint8_t byte_to_embed = bytes_to_embed[i];
        for (int bit_position = 0; bit_position < bytes_to_embed_byte; bit_position += 1) {
            // In LSBI we don't alter color RED
            if (lsbi) {
                const color past_color = *current_color;
                *current_color = get_next_color(past_color);
                if (past_color == RED) {
                    it_buffer++;
                    bit_position--;
                    continue;
                }
            }

            // Embed the byte
            const uint8_t offset = 8 - ((bit_position + 1) * n);
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
