#include "include/get_file_size.h"
#include "include/print_error.h"

#include <sys/stat.h>

off_t get_file_size(const char *filepath) {
    struct stat st;

    // Get file information
    if (stat(filepath, &st) == 0) {
        return st.st_size;
    }

    print_error("Error getting file information");
    return -1;
}
