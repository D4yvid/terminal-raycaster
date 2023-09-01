#ifndef RENDERER_H
#define RENDERER_H

#include "types.h"

#define PIXEL_CHAR L'▀'

#define SET_ALTERNATE_BUFFER "\033[?1049h"
#define RESTORE_ORIGINAL_BUFFER "\033[?1049l"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#define CLEAR_SCREEN "\033[H\033[2J\033[3J"

#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4

#define WRITE(...) printf(__VA_ARGS__); fflush(stdout)

struct Screen {
	i32 width, height;
	u8 *currentBuffer;
	u8 *backBuffer, *frontBuffer;
};

void renderer_init(void);

struct Screen *renderer_get_screen(void);

void renderer_get_size(i32 *width, i32 *height);
void renderer_resize(void);

void renderer_clear(void);

void renderer_pixelat(i32 x, i32 y, u8 color);

void renderer_swap(void);

void renderer_destroy(void);

#endif /** RENDERER_H */
