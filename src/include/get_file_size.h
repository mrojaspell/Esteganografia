#ifndef GET_FILE_SIZE_H
#define GET_FILE_SIZE_H
#include <sys/types.h>  // For off_t

#define MAX_EXTENSION_SIZE 10

#define BMP_HEADER_SIZE 54
#define SECRET_SIZE_BYTES 4
#define SECRET_SIZE_IN_COVER_LSB1(X) (X*8)
#define SECRET_SIZE_IN_COVER_LSB4(X) (X*2)


// Get the size of a file in bytes, using the stat system call
off_t get_file_size(const char * filepath);

int get_file_extension(const char *filepath, char *extension);

#endif