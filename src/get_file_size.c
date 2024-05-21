#include <stdlib.h>

#include "include/get_file_size.h"
#include "include/color_print.h"

#include <stdio.h>
#include <sys/stat.h>

off_t get_file_size(const char *filepath) {
    struct stat st;

    // Get file information
    if (stat(filepath, &st) == 0) {
        return st.st_size;
    } else {
        color_print(stderr, RED, "Error getting file information");
        return -1;
    }
}