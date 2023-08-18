#include "renderer.h"
#include "types.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define PUTPIXEL() printf("%lc", PIXEL_CHAR)

__INTERNAL__
struct termios term_state;

__INTERNAL__
struct Screen screen;

struct Screen *renderer_get_screen(void)
{
	return &screen;
}

void renderer_get_size(i32 *width, i32 *height)
{
	struct winsize sz;

	ioctl(STDIN_FILENO, TIOCGWINSZ, &sz);

	*width = sz.ws_col;
	*height = sz.ws_row;
}

void renderer_resize(void)
{
	int width = 0, height = 0;

	renderer_get_size(&width, &height);

	screen.width = width;
	screen.height = height * 2;
	screen.buffer = realloc(screen.buffer, sizeof(*screen.buffer) * width * height);

	WRITE(CLEAR_SCREEN);
}

void renderer_init(void)
{
	struct termios raw;

	renderer_resize();
	tcgetattr(STDIN_FILENO, &term_state);

	raw = term_state;

	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 0;

	tcsetattr(STDIN_FILENO, TCSANOW, &raw);

	WRITE(HIDE_CURSOR);
	WRITE(SET_ALTERNATE_BUFFER);
}

void renderer_clear(void)
{
	memset(screen.buffer, 0, sizeof(*screen.buffer) * screen.width * screen.height);

	WRITE("\033[;H");
}

void renderer_pixelat(i32 x, i32 y, u8 color)
{
	if (x < 0 || x >= screen.width ||
		y < 0 || y >= (screen.height )) return;

	screen.buffer[(y * screen.width) + x] = color;
}

void renderer_draw(void)
{
	printf("\033[1m");

	for (int y1 = 0; y1 < screen.height; y1 += 2) {
		int y2 = y1 + 1;

		for (int x = 0; x < screen.width; x++) {
			if (screen.buffer[y1 * screen.width + x])
				printf("\033[3%dm", screen.buffer[y1 * screen.width + x]);
			else
				printf("\033[30m");

			if (y2 < screen.height && screen.buffer[y2 * screen.width + x])
				printf("\033[4%dm", screen.buffer[y2 * screen.width + x]);
			else
				printf("\033[40m");

			PUTPIXEL();
		}

		if (y1 + 4 > screen.height)
			printf("\033[;H");
		else
			printf("\n\r");
	}

	printf("\033[0m");
	fflush(stdout);
}

void renderer_destroy(void)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &term_state);

	WRITE(RESTORE_ORIGINAL_BUFFER);
	WRITE(SHOW_CURSOR);

	free(screen.buffer);
}

