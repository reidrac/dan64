/*
 * vidmem.c
 *
 * Example using direct access to video memory.
 *
 * This example only uses DAN64 syscalls.
 */

#include "d64.h"

int
main()
{
	register uint8_t *vidmem = (uint8_t *)0x0200;
	register uint8_t i, j;

	gotoxy(0, 20);
	cputs("Inverting the screen using\ndirect memory access.\n");

	for (j = 0; j < 192; j++)
		for (i = 0; i < 32; i++)
			vidmem[i + j * 32] ^= 0xff;

	return 0;
}

