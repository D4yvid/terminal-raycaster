#ifndef RENDERER_H
#define RENDERER_H

#include "types.h"

#define HEIGHT 24
#define WIDTH  24

#define PIXEL_CHAR L'▀'

#define SET_ALTERNATE_BUFFER "\033[?1049h"
#define RESTORE_ORIGINAL_BUFFER "\033[?1049l"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4

#define WRITE(...) printf(__VA_ARGS__); fflush(stdout)

void renderer_init(void);

void renderer_clear(void);
void renderer_pixelat(i32 x, i32 y, i32 color);

void renderer_draw(void);

void renderer_destroy(void);

#endif /** RENDERER_H */
