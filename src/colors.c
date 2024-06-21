#include "colors.h"

// The order in which colors are read from the file are BLUE, GREEN, RED
// This function returns the next color to read, given current_rgb_color
color get_next_color(color current_rgb_color){
    if (current_rgb_color == BLUE)
        return GREEN;
    else if (current_rgb_color == GREEN)
        return RED;
    else // (current_rgb_color == RED)
        return BLUE;
}