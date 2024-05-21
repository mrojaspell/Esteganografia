#include "include/color_print.h"


static char * colors[] = {
    "\033[0m",
    "\x1b[32m",
    "\x1b[33m",
    "\033[31m"
};



void color_print(FILE * fd, int color,const char* format, ...) {
    va_list args;
    va_start(args, format);

    // Print the formatted string to stderr with red color
    fprintf(fd, "%s", colors[color]); // Set text color to red
    vfprintf(fd, format, args);        // Print the formatted string
    fprintf(fd, "%s", colors[RESET]); // Reset text color

    va_end(args);
}
