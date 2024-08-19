#include <stdio.h>
#include <unistd.h>

#include "tty.h"

int main()
{
	tty_setup();
	tty_clear();
	tty_flush();

	printf("setup\n");
	fflush(stdin);
	char tmp = 0;
	while (tmp != 'q')
	{
		read(STDIN_FILENO, &tmp, 1);
		printf("%d\n", tmp);
	}
}

