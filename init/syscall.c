/*
 * syscall.c (DAN64 syscall implementation)
 * Copyright (C) 2015 by Juan J. Martinez <jjm@usebox.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
*/

#include "hardware.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "init.h"
#include "memory.h"
#include "vm.h"
#include "video.h"
#include "keyboard.h"

#include "strings.h"

extern uint8_t buffer[];
extern uint8_t prog_exit;
// cursor position
extern uint8_t x, y;

// VM registers used by syscall
extern uint8_t r_sp, r_a;

// use local SRAM for zp and hardware stack
static uint8_t local[512];

void
vm_ram_read(uint16_t addr, uint8_t *dst, uint8_t size)
{
	uint16_t part = 0;

	if (!size)
		return;

	if (addr < 512)
	{
		if (size + addr > 512)
			part = 512 - addr;
		else
			part = size;

		memcpy(dst, local + addr, part);
	}

	if (size)
	{
		addr += part;
		dst += part;
		size -= part;

		sram_read(addr, dst, size);
	}
}

void
vm_ram_write(uint16_t addr, uint8_t *src, uint8_t size)
{
	uint16_t part = 0;

	if (!size)
		return;

	if (addr < 512)
	{
		if (size + addr > 512)
			part = 512 - addr;
		else
			part = size;

		memcpy(local + addr, src, part);
	}

	if (size)
	{
		addr += part;
		src += part;
		size -= part;

		sram_write(addr, src, size);
	}
}

void
vm_syscall(uint8_t func)
{
	uint8_t v[6];
	uint16_t addr, count, fd, size, i;

	switch(func)
	{
		case 0x00:
			// terminate program
			//  in: exit code
			// ret: -
			prog_exit = 1;
			vm_ram_read(addr16(r_sp + 1, 1), v, 1);
			if (*v)
			{
				strcpy_P((char *)buffer, text_err_prg);
				put_string((const char *)buffer, *v);
			}
			else
			{
				strcpy_P((char *)buffer, text_err_ok);
				put_string((const char *)buffer);
			}
			break;
		case 0x01:
			// load data
			//  in: addr to destination
			// ret: 0 on success
			vm_ram_read(addr16(r_sp + 1, 1), v, 2);
			addr = addr16(v[1], v[0]);
			// quiet = 1, suppress error output
			r_a = load(addr, 1);
			break;
		case 0x02:
			// save data
			//  in: addr of start, number bytes to save
			// ret: 0 on success
			vm_ram_read(addr16(r_sp + 1, 1), v, 4);
			addr = addr16(v[1], v[0]);
			count = addr16(v[3], v[2]);
			// quiet = 1, suppress error output
			r_a = save(addr, addr + count, 1);
			break;
		case 0x10:
			// put char
			//  in: character
			// ret: 0 on success
			vm_ram_read(addr16(r_sp + 1, 1), v, 1);
			video_put_char(x, y, *v);
			r_a = 0;
			break;
		case 0x11:
			// put string
			//  in: addr to zstring
			// ret: number of printed characters
			vm_ram_read(addr16(r_sp + 1, 1), v, 2);
			addr = addr16(v[1], v[0]);
			r_a = 0;
			while(1)
			{
				vm_ram_read(addr++, v, 1);
				if (!*v)
					break;
				put_char(*v);
				r_a++;
			}
			break;
		case 0x12:
			// set cursor position
			//  in: x, y
			// ret: 0 on success
			vm_ram_read(addr16(r_sp + 1, 1), v, 2);
			if (v[0] > 31 || v[1] > 23)
				r_a = 1;
			else
			{
				r_a = 0;
				x = v[0];
				y = v[1];
			}
			break;
		case 0x13:
			// fill screen
			//  in: character
			// ret: 0 on success
			vm_ram_read(addr16(r_sp + 1, 1), v, 1);
			video_cls(*v);
			r_a = x = y = 0;
			break;
		case 0x14:
			// write
			//  in: fd, buffer addr, count
			// ret: bytes written, 0 on error
			r_a = 0;
			vm_ram_read(addr16(r_sp + 1, 1), v, 6);
			fd = addr16(v[1], v[0]);
			addr = addr16(v[3], v[2]);
			count = addr16(v[5], v[4]);

			// only stdout and stderr
			if (fd > 0 && fd < 3 && count)
			{
				r_a += count;
				while (count)
				{
					size = count < 64 ? count : 64;
					vm_ram_read(addr, buffer, size);
					count -= size;
					addr += size;
					for (i = 0; i < size; i++)
						put_char(buffer[i]);
				}
			}
			break;
		case 0x20:
			// get char
			//  in: -
			// ret: character ascii or 0
			r_a = keyboard_asc();
			break;
		case 0x21:
			// get input
			//  in: addr to the buffer, size of the buffer
			// ret: number of characters read
			//
			// The size of the buffer is limited to 64 chars
			vm_ram_read(addr16(r_sp + 1, 1), v, 3);
			addr = addr16(v[1], v[0]);
			r_a = buffered_input(buffer, v[2] < 64 ? v[2] : 64);
			vm_ram_write(addr, buffer, r_a);
			break;
		case 0x22:
			// read
			//  in: fd, buffer addr, count
			// ret: bytes read, 0 on error
			r_a = 0;
			vm_ram_read(addr16(r_sp + 1, 1), v, 6);
			fd = addr16(v[1], v[0]);
			addr = addr16(v[3], v[2]);
			count = addr16(v[5], v[4]);

			// only stdin
			if (fd == 0 && count)
			{
				while (count)
				{
					video_wait();
					*v = keyboard_asc();
					if (*v)
					{
						put_char(*v);
						if (*v == 0x0a)
							*v = '\n';
						vm_ram_write(addr++, v, 1);
						count--;
						r_a++;
					}
				}
			}
			break;
		case 0x30:
			// put tile
			//  in: addr to tile definition (8 bytes)
			// ret: 0 on success
			vm_ram_read(addr16(r_sp + 1, 1), v, 2);
			addr = addr16(v[1], v[0]);
			vm_ram_read(addr, buffer, 8);
			video_put_tile(x, y, buffer);
			break;
		case 0xa0:
			// get random
			//  in: -
			// ret: random byte
			r_a = rand() & 0xff;
			break;
		case 0xa1:
			// wait for vsync
			//  in: -
			// ret: 0 on success
			video_wait();
			break;
		case 0xa2:
			// srand
			//  in: random seed
			// ret: 0 on success
			vm_ram_read(addr16(r_sp + 1, 1), v, 2);
			srand(addr16(v[1], v[0]));
			r_a = 0;
			break;
		case 0xf0:
			// get version
			//  in: _
			// ret: version (x.y as (x | (y << 4)))
			r_a = 1;
			break;
		default:
			prog_exit = 1;
			strcpy_P((char *)buffer, text_err_sys);
			put_string((char *)buffer, func);
			break;
	}
}

