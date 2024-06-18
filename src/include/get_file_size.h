#ifndef GET_FILE_SIZE_H
#define GET_FILE_SIZE_H
#include <sys/types.h>  // For off_t

// Max length we allow for file extensions
#define MAX_EXTENSION_SIZE 10

// Size in bytes of BMP header
#define BMP_HEADER_SIZE 54
#define SECRET_SIZE_BYTES 4

// The bytes to store the payload size in OUT file
#define FILE_LENGTH_BYTES 4 

#define SECRET_SIZE_IN_COVER_LSB1(X) (X*8)
#define SECRET_SIZE_IN_COVER_LSB4(X) (X*2)

// The number of bytes needed to embed a byte
#define BYTES_TO_EMBED_BYTE(n) (8/n)

// The number of bytes in P necessary to embed IN file size
#define SECRET_SIZE_IN_COVER_LSBN(n) (FILE_LENGTH_BYTES * BYTES_TO_EMBED_BYTE(n))


// Get the size of a file in bytes, using the stat system call
off_t get_file_size(const char * filepath);

// Copy file extension to buffer. Returns size in bytes.
int get_file_extension(const char *filepath, char *extension);

#endif