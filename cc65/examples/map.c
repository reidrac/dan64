/*
 * map.c
 *
 * This example only uses DAN64 syscalls.
 */

#include "d64.h"

int
main()
{
	register uint8_t r;
	char buffer[2] = { 0, 0 };

	clrscr();

	while(1)
	{
		// module is slow, using AND instead
		r = _rand() & 3;
		// cputs does move the cursor
		buffer[0] = 176 + (r > 2 ? 1 : r);
		cputs(buffer);
	}

	return 0;
}

