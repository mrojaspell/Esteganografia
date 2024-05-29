#include "include/get_file_size.h"

#include <string.h>

#include "include/print_error.h"

#include <sys/stat.h>

off_t get_file_size(const char* filepath) {
    struct stat st;

    // Get file information
    if (stat(filepath, &st) == 0) {
        return st.st_size;
    }

    print_error("Error getting file information");
    return -1;
}

int get_file_extension(const char* filepath, char* extension) {
    const char* dot = strrchr(filepath, '.');
    if (dot == NULL) {
        *extension = '\0';
        return 0;
    }
    int i = 0;
    for (; dot[i]; i++) {
        extension[i] = dot[i];
    }
    extension[i] = '\0';
    return i;
}
