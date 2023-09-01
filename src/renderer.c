#include "renderer.h"
#include "types.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define PUTPIXEL() printf("%lc", PIXEL_CHAR)
#define GETXY(b,x,y) b[y * screen.width + x]

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
	screen.frontBuffer = realloc(screen.frontBuffer, sizeof(*screen.frontBuffer) * width * height);
	screen.backBuffer = realloc(screen.backBuffer, sizeof(*screen.backBuffer) * width * height);

	screen.currentBuffer = screen.backBuffer;

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
	memset(screen.currentBuffer, 0, sizeof(*screen.currentBuffer) * screen.width * screen.height);

	WRITE("\033[;H");
}

void renderer_pixelat(i32 x, i32 y, u8 color)
{
	if (x < 0 || x >= screen.width ||
		y < 0 || y >= screen.height) return;

	screen.currentBuffer[(y * screen.width) + x] = color;
}

void renderer_swap(void)
{
	u8 *cmpBuf = screen.currentBuffer == screen.backBuffer ? screen.frontBuffer : screen.backBuffer;
	u8 *curBuf = screen.currentBuffer;

	printf("\033[1m");

	for (i32 y1 = 0; y1 < screen.height; y1 += 2) {
		i32 y2 = y1 + 1;
		i32 col = 0;

		for (i32 x = 0; x < screen.width; x++) {
			u8 changed = 0;

			if (GETXY(curBuf, x, y1) != GETXY(cmpBuf, x, y1) ||
				GETXY(curBuf, x, y2) != GETXY(cmpBuf, x, y2))
				changed = 1;
			
			else
				col++;

			if (changed) {
				if (col) printf("\033[%dC", col);

				if (GETXY(curBuf, x, y1))
					printf("\033[3%dm", GETXY(curBuf, x, y1));
				else
					printf("\033[30m");
				
				if (y2 < screen.height && GETXY(curBuf, x, y2))
					printf("\033[4%dm", GETXY(curBuf, x, y2));
				else
					printf("\033[40m");
				
				PUTPIXEL();

				col = 0;
			}
		}

		if (y1 + 4 > screen.height)
			printf("\033[;H");
		else
			printf("\033[%d;H", (y1 / 2) + 2);
	}

	printf("\033[0m");
	fflush(stdout);

	screen.currentBuffer = cmpBuf;
}

void renderer_destroy(void)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &term_state);

	WRITE(RESTORE_ORIGINAL_BUFFER);
	WRITE(SHOW_CURSOR);

	free(screen.frontBuffer);
	free(screen.backBuffer);
}

