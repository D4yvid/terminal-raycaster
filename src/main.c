#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include "types.h"
#include "renderer.h"

typedef enum {
	QUIT_REASON_SIGNAL,
	QUIT_REASON_DEADLY_SIGNAL,
	QUIT_REASON_PROGRAM_FINISH
} QuitReason;

i32 SIGNALS[] = {
	SIGHUP, SIGINT, SIGILL, SIGTRAP, SIGABRT,
	SIGBUS, SIGFPE, SIGUSR1, SIGSEGV, SIGUSR2,
	SIGPIPE, SIGALRM, SIGTERM, SIGSTKFLT, SIGCHLD, SIGCONT,
	SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU, SIGURG, SIGXCPU,
	SIGXFSZ, SIGVTALRM, SIGPROF, SIGWINCH, SIGIO, SIGPWR,
	SIGSYS
};

void init(void);

void onexit(void);
void onsignal(i32 sig);

void __NORETURN__ deadly_signal(i32 sig);

void quit(QuitReason reason, i32 code, void *data);

void onexit(void)
{
	quit(QUIT_REASON_PROGRAM_FINISH, 0, NULL);
}

void __NORETURN__ deadly_signal(i32 sig)
{
	quit(QUIT_REASON_DEADLY_SIGNAL, sig, &sig);

	while (1);
}

void onsignal(i32 sig)
{
	switch (sig)
	{
	case SIGWINCH:
		renderer_resize();
		return;

	case SIGSEGV:
	case SIGILL:
	case SIGFPE:
	case SIGTERM:
	case SIGHUP:
	case SIGSYS:
		deadly_signal(sig);
		return;

	case SIGINT:
		quit(QUIT_REASON_SIGNAL, 1, &sig);
		return;
	}
}

void init(void)
{
	atexit(onexit);

	for (int i = 0; i < sizeof(SIGNALS) / sizeof(SIGNALS[0]); i++) {
		signal(SIGNALS[i], onsignal);
	}

	renderer_init();
}

void __NORETURN__ quit(QuitReason reason, i32 code, void *data)
{
	renderer_destroy();

	switch (reason)
	{
	case QUIT_REASON_SIGNAL:
	case QUIT_REASON_PROGRAM_FINISH: break;
	case QUIT_REASON_DEADLY_SIGNAL:
		WRITE("Caught deadly signal: %d\n", *((i32 *) data));
		break;
	}

	printf("\n");
	exit(code);
}

i32 main(void)
{
	u8 c;
	i32 n = 1;
	struct Screen *screen;

	init();

	screen = renderer_get_screen();

	while (1)
	{
		while (read(STDIN_FILENO, &c, 1) && c);

		if (c == 'q')
			break;

		renderer_clear();

		renderer_pixelat(n % screen->width, n % screen->height, 7);

		renderer_draw();

		usleep(MS_TO_US(1000 / 60));

		n++;
	}

	return 0;
}

