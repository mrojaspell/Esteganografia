#ifndef GET_FILE_SIZE_H
#define GET_FILE_SIZE_H
#include <status_codes.h>
#include <sys/types.h>  // For off_t
#include <stdio.h>
#include <math.h>

// Max length we allow for file extensions
#define MAX_EXTENSION_SIZE 10

#define BITS_IN_BYTES 8

// Size in bytes of BMP header
#define BMP_OFFSET_POSITION 10
#define BMP_OFFSET_SIZE 4

#define BMP_HEADER_SIZE 54
#define SECRET_SIZE_BYTES 4

// The bytes to store the payload size in OUT file
#define FILE_LENGTH_BYTES 4
#define FILE_LENGTH_BITS FILE_LENGTH_BYTES * 8

#define SECRET_SIZE_IN_COVER_LSB1(X) ((X)*8)
#define SECRET_SIZE_IN_COVER_LSB4(X) ((X)*2)
#define SECRET_SIZE_IN_COVER_LSBI(X) ((X)*8*3/2)

// The number of bytes needed to embed a byte in LSBN
#define BYTES_TO_EMBED_BYTE_LSBN(n) (8/n)

// The number of bytes needed to embed a byte in LSBI
// We divide by 2/3 because we are using 2 colors (BLUE and GREEN) of 3 in RGB
#define BYTES_TO_EMBED_BYTE_LSBI(n) ((BYTES_TO_EMBED_BYTE_LSBN(n) * 3) / 2)


// Get the size of a file in bytes, using the stat system call
off_t get_file_size(const char* filepath);

// Copy file extension to buffer. Returns size in bytes.
int get_file_extension(const char* filepath, char* extension);
status_code copy_bmp_header_file_to_file(FILE* copy_from, FILE* copy_to);
status_code skip_bmp_header(FILE* file);
status_code copy_from_file_to_file(FILE* copy_from, FILE* copy_to, unsigned int len);
status_code copy_rest_of_file(FILE* copy_from, FILE* copy_to);
status_code get_bmp_header_size(FILE * file, uint32_t * size);
#endif
