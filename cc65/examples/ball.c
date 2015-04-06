/*
 * ball.c
 *
 * Bouncing ball example using putt (put tile).
 *
 * This example only uses D64 syscalls.
 */

#include "d64.h"

// ball tile
uint8_t ball[] = { 0, 0x18, 0x3c, 0x7e, 0x7e, 0x3c, 0x18, 0 };

void
delay()
{
	int i;

	// vsync happens 50 times per second, so this waits
	// for around 1/3 a second
	for (i = 0; i < 50/3; i++)
		wait_vsync();
}

int
main()
{
	register char x, y, ix, iy, i;

	x = 1 + _rand() % 30;
	y = 1 + _rand() % 21;

	ix = iy = 1;

	clrscr();

	// make a frame
	for (i = 0; i < 32; i++)
	{
		gotoxy(i, 0);
		putch(196);
		gotoxy(i, 23);
		putch(196);
	}
	for (i = 0; i < 24; i++)
	{
		gotoxy(0, i);
		putch(179);
		gotoxy(31, i);
		putch(179);
	}

	while(1)
	{
		gotoxy(x, y);
		putt(ball);

		if (x + ix > 30 || x + ix < 1)
			ix *= -1;
		else
			x += ix;

		if (y + iy > 22 || y + iy < 1)
			iy *= -1;
		else
			y += iy;

		// if we do it too fast, we'll see a blinking ball
		delay();
		putch(' ');
	}

	return 0;
}

