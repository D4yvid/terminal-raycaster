#include "renderer.h"
#include "types.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#define PUTPIXEL() printf("%lc", PIXEL_CHAR)

__INTERNAL__
struct termios term_state;

__INTERNAL__
u8 FRAMEBUFFER[HEIGHT][WIDTH];

void renderer_init(void)
{
	tcgetattr(STDIN_FILENO, &term_state);

	struct termios raw = term_state;

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
	memset(FRAMEBUFFER, 0, sizeof(FRAMEBUFFER));

	WRITE("\033[;H");
}

void renderer_pixelat(i32 x, i32 y, i32 color)
{
	if (x < 0 || x >= WIDTH ||
		y < 0 || y >= HEIGHT) return;

	FRAMEBUFFER[y][x] = color;
}

void renderer_draw(void)
{
	for (int y1 = 0; y1 < HEIGHT; y1 += 2) {
		int y2 = y1 + 1;

		for (int x = 0; x < WIDTH; x++) {
			if (FRAMEBUFFER[y1][x])
				printf("\033[3%dm", FRAMEBUFFER[y1][x]);
			else
				printf("\033[30m");

			if (y2 < HEIGHT && FRAMEBUFFER[y2][x])
				printf("\033[4%dm", FRAMEBUFFER[y2][x]);
			else
				printf("\033[40m");

			PUTPIXEL();

			printf("\033[0m");
		}

		printf("\r\n");
	}
}

void renderer_destroy(void)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &term_state);

	WRITE(RESTORE_ORIGINAL_BUFFER);
	WRITE(SHOW_CURSOR);
}

