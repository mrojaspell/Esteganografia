#include <stdio.h>
#include <stdarg.h> // Include the header for variadic functions

#include "include/print_error.h"

// Define ANSI escape codes for red text and reset
#define ANSI_COLOR_RED "\033[31m"
#define ANSI_COLOR_RESET "\033[0m"

void print_error(const char* format, ...) {
    va_list args;
    va_start(args, format);

    // Print the formatted string to stderr with red color
    fprintf(stderr, "%s", ANSI_COLOR_RED); // Set text color to red
    vfprintf(stderr, format, args);        // Print the formatted string
    fprintf(stderr, "%s", ANSI_COLOR_RESET); // Reset text color

    va_end(args);
}
