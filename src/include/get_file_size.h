#ifndef GET_FILE_SIZE_H
#define GET_FILE_SIZE_H
#include <sys/types.h>  // For off_t
#include <sys/stat.h>   // For struct stat and stat

off_t get_file_size(const char * filepath);

#endif