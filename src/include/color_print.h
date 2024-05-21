#ifndef COLOR_ERROR_H
#define COLOR_ERROR_H

#include <stdio.h>
#include <stdarg.h> 

typedef enum colors {
    RESET,
    GREEN,
    YELLOW,
    RED
} tcolors;


void color_print(FILE * fd, int color, const char * fmt, ...);

#endif
