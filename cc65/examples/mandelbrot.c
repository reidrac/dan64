/*
 * mandelbrot.c
 *
 * Mandelbrot set in ASCII for DAN64.
 *
 * Uses fixed point arithmetic (6.10) with signed 16-bit numbers.
 *
 */

#include "d64.h"

#define SHIFT 			10
#define fp(a) 			(((int16_t)a) << SHIFT)
#define fpmul(a, b)		(((((int32_t)a) * (b))) >> SHIFT)

int
main()
{
	register int8_t x, y;
	register int16_t ca, cb, a, b;
	int16_t t;
	uint8_t i;
	char c;

	for (y = -11; y < 12; y++)
	{
		for (x = -19; x < 12; x++)
		{
			ca = fpmul(fp(x), 43);
			cb = fpmul(fp(y), 85);
			a = ca;
			b = cb;
			for (i = 0; i < 12; i++)
			{
				t = fpmul(a, a) - fpmul(b, b) + ca;
				b = fpmul(fpmul(fp(2), a), b) + cb;
				a = t;
				if (fpmul(a, a) + fpmul(b,b) > fp(4))
				{
					if (i > 9)
						i += 7;
					c = 48 + i;
					break;
				}
			}
			if (i == 12)
				c = ' ';

			gotoxy(x + 19, y + 11);
			putch(c);
		}
	}

	// wait for enter
	while (getch() != 0x0a);

	gotoxy(0, 0);

	return 0;
}
