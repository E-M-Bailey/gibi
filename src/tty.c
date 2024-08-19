#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "tty.h"

struct termios original_mode;

void restore_tty()
{
	tcsetattr(STDIN_FILENO, TCSANOW, &original_mode);
}

void tty_setup()
{
	tcgetattr(STDIN_FILENO, &original_mode);
	atexit(restore_tty);

	struct termios raw_mode = original_mode;
	// Set the terminal to raw mode. This is like cfmakeraw, but CR/LF etc. processing is left as-is.
	// (This may change in the future if exact CR/LF is needed for some reason.)
	// see `man termios(3)`.
	raw_mode.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | IXON);
	raw_mode.c_lflag &= ~(ISIG | ICANON | ECHO | ECHONL);
	raw_mode.c_cflag &= ~(CSIZE | PARENB);
	raw_mode.c_cflag |= CS8;
	
	tcsetattr(STDIN_FILENO, TCSANOW, &raw_mode);
}

void tty_cursor_position(int row, int col)
{
	printf("\e[%d;%dH", row + 1, col + 1);
}
void tty_clear()
{
	printf("\e[2J\e[H");
}

void tty_flush()
{
	fflush(stdout);
}

