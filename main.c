#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define HEIGHT 24
#define WIDTH  24

#define PIXEL_CHAR L'▀'

#define SET_ALTERNATE_BUFFER "\033[?1049h"
#define RESTORE_ORIGINAL_BUFFER "\033[?1049l"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

#define PUTPIXEL() printf("%lc", PIXEL_CHAR)
#define WRITE(...) printf(__VA_ARGS__); fflush(stdout)

#define MS_TO_US(ms) ms * 1000

#define __NORETURN__ __attribute__((noreturn))

#define FOREACH_PIXEL(_x, _y)				\
	for (int _y##1 = 0, _y##2 = 1; _y##1 < HEIGHT; _y##1++, _y##2++)	\
		for (int _x = 0; _x < WIDTH; _x++)

typedef char i8;
typedef short i16;
typedef int i32;
typedef long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef float f32;
typedef double f64;

typedef enum {
	QUIT_REASON_SIGNAL,
	QUIT_REASON_DEADLY_SIGNAL,
	QUIT_REASON_PROGRAM_FINISH
} QuitReason;

u8 FRAMEBUFFER[HEIGHT][WIDTH];

i32 SIGNALS[] = {
	SIGHUP, SIGINT, SIGILL, SIGTRAP, SIGABRT,
	SIGBUS, SIGFPE, SIGUSR1, SIGSEGV, SIGUSR2,
	SIGPIPE, SIGALRM, SIGTERM, SIGSTKFLT, SIGCHLD, SIGCONT,
	SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU, SIGURG, SIGXCPU,
	SIGXFSZ, SIGVTALRM, SIGPROF, SIGWINCH, SIGIO, SIGPWR,
	SIGSYS
};

struct termios term_state;

void clear(void);
void render(void);

void init(void);

void onexit(void);
void onsignal(i32 sig);

void __NORETURN__ deadly_signal(i32 sig);

void quit(QuitReason reason, void *data);

void clear(void)
{
	memset(FRAMEBUFFER, 0, sizeof(FRAMEBUFFER));

	WRITE("\033[;H");
}

void render(void)
{
	for (int y1 = 0; y1 < HEIGHT; y1 += 2) {
		int y2 = y1 + 1;

		for (int x = 0; x < WIDTH; x++) {
			if (FRAMEBUFFER[y1][x])
				printf("\033[38;2;255;255;255m");
			else
				printf("\033[38;2;0;0;0m");

			if (y2 < HEIGHT && FRAMEBUFFER[y2][x])
				printf("\033[48;2;255;255;255m");
			else
				printf("\033[48;2;0;0;0m");

			PUTPIXEL();

			printf("\033[0m");
		}

		printf("\r\n");
	}
}

void onexit(void)
{
	quit(QUIT_REASON_PROGRAM_FINISH, NULL);
}

void __NORETURN__ deadly_signal(i32 sig)
{
	quit(QUIT_REASON_DEADLY_SIGNAL, &sig);

	while (1);
}

void onsignal(i32 sig)
{
	switch (sig)
	{
	case SIGSEGV:
	case SIGILL:
	case SIGFPE:
	case SIGTERM:
	case SIGHUP:
	case SIGSYS:
		deadly_signal(sig);
	case SIGINT:
		quit(QUIT_REASON_SIGNAL, &sig);
	}
}

void init(void)
{
	atexit(onexit);

	for (int i = 0; i < sizeof(SIGNALS) / sizeof(SIGNALS[0]); i++) {
		signal(SIGNALS[i], onsignal);
	}

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

void __NORETURN__ quit(QuitReason reason, void *data)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &term_state);

	WRITE(RESTORE_ORIGINAL_BUFFER);
	WRITE(SHOW_CURSOR);

	switch (reason)
	{
	case QUIT_REASON_SIGNAL:
	case QUIT_REASON_PROGRAM_FINISH: break;
	case QUIT_REASON_DEADLY_SIGNAL:
		WRITE("Caught deadly signal: %d\n", *((int *) data));
		break;
	}

	printf("\n");
	exit(-1);
}

i32 main(void)
{
	init();

	while (1)
	{
		clear();

		FRAMEBUFFER[10][5] = 1;
		FRAMEBUFFER[11][3] = 2;

		render();

		usleep(MS_TO_US(100));
	}

	return 0;
}

