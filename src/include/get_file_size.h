#ifndef GET_FILE_SIZE_H
#define GET_FILE_SIZE_H
#include <sys/types.h>  // For off_t

#define BMP_HEADER_SIZE 54

// Get the size of a file in bytes, using the stat system call
off_t get_file_size(const char * filepath);

#endif