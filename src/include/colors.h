#ifndef COLORS_H
#define COLORS_H

typedef enum color {
    RED,
    GREEN,
    BLUE
} color;

color get_next_color(color current_rgb_color);

#endif