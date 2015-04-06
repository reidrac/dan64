/*
 * main.c (main entry point)
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
#include <ctype.h>

#include "init.h"
#include "memory.h"
#include "vm.h"
#include "video.h"
#include "keyboard.h"
#include "storage.h"
#include "dasm.h"

#define _INIT_C
#include "strings.h"

uint8_t buffer[128];

uint8_t prog_exit = 0;
// cursor position
uint8_t x = 0, y = 0;

void
scroll_up()
{
	uint8_t b[64];
	uint16_t i, j, src = VIDEO_ADDR + CHARS_WIDTH * 8, dst = VIDEO_ADDR;

	video_wait();

	for (i = 0; i < CHARS_HEIGHT - 1; i++)
	{
		for (j = 0; j < 4; j++)
		{
			sram_read(src + j * (CHARS_WIDTH << 1), b, CHARS_WIDTH << 1);
			sram_write(dst + j * (CHARS_WIDTH << 1), b, CHARS_WIDTH << 1);
		}
		src += CHARS_WIDTH * 8;
		dst += CHARS_WIDTH * 8;
	}
	for (j = 0; j < 4; j++)
		sram_set(dst + j * (CHARS_WIDTH << 1), 0, CHARS_WIDTH << 1);
}

uint8_t
buffered_input(uint8_t *buffer, uint8_t size)
{
	uint8_t pos = 0, end = 0, d, i, j, k;
	uint16_t cnt = 0;

	while (1)
	{
		video_wait();
		d = keyboard_asc();
		switch(d)
		{
            case 0: // update the cursor
                if (++cnt == 0x4000)
                {
                    video_cursor(x, y);
                    cnt = 0;
                }
                break;

			case 0x11: // left
				video_cursor_off(x, y);
				if (pos > 0)
				{
					pos--;
					if (x > 0)
						x--;
					else
					{
						y--;
						x = 31;
					}
				}
				break;

			case 0x10: // right
				video_cursor_off(x, y);
				if (pos < end)
				{
					pos++;
					if (x < 31)
						x++;
					else
					{
						x = 0;
						y++;
					}
				}
				break;

            case 0x0a: // new line
				video_cursor_off(x, y);
				x = 0;
				y++;
				if (y > 23)
				{
					scroll_up();
					y--;
				}

				// return a zstring
				buffer[end] = 0;
				return end;

            case 0x08: // backspace
				video_cursor_off(x, y);

				if (pos > 0)
				{
					pos--;

					if (x > 0)
						x--;
					else
					{
						x = 31;
						y--;
					}

					i = x;
					j = y;

					for (k = pos; k < end; k++)
					{
						if (k < end - 1)
						{
							video_put_char(i, j, buffer[k + 1]);
							buffer[k] = buffer[k + 1];
						}
						else
							video_put_char(i, j, ' ');

						if (i < 31)
							i++;
						else
						{
							i = 0;
							j++;
						}
					}

					end--;
				}
                break;

            default:
				video_cursor_off(x, y);

				// buffer is full
				if (end + 1 == size)
					break;

				if (pos == end)
				{
					buffer[pos++] = d;
					end = pos;

					video_put_char(x, y, d);
					if (x < 31)
						x++;
					else
					{
						x = 0;
						y++;
						if (y > 23)
						{
							scroll_up();
							y--;
						}
					}
				}
				else
				{
					end++;
					i = x + (end - 1 - pos);
					j = y;

					if (i > 31)
					{
						i -= 32;
						j++;
						if (j > 23)
						{
							scroll_up();
							j--;
							y--;
						}
					}

					for (k = end - 1; k != pos; k--)
					{
						video_put_char(i, j, buffer[k - 1]);
						buffer[k] = buffer[k - 1];

						if (i > 0)
							i--;
						else
						{
							i = 31;
							j--;
						}

					}

					buffer[pos] = d;
					video_put_char(x, y, d);
				}

                break;
		}
	}
}

void
put_char(char c)
{
	switch(c)
	{
		default:
			video_put_char(x++, y, c);
			break;
		case 0x0a:
			// new line
			x = 0;
			y++;

			if (y > 23)
			{
				scroll_up();
				y--;
			}
			break;
		case 0x09:
			// tab
			x = ((x / 8) + 1) * 8;
			break;
	}

	if (x > 31)
	{
		x = 0;
		y++;
		if (y > 23)
		{
			scroll_up();
			y--;
		}
	}
}

void
put_string(const char *fmt, ...)
{
	va_list args;
	char b[128];
	uint8_t i;

    va_start(args, fmt);
	vsnprintf((char *)b, 128, fmt, args);
	va_end(args);

	for(i = 0; b[i]; i++)
		put_char(b[i]);
}

void
load_data_write(uint8_t byte, void *arg)
{
	uint16_t *addr = (uint16_t *)arg;

	sram_write(*addr, &byte, 1);
	(*addr)++;
}

uint8_t
load(uint16_t dest_addr, uint8_t quiet)
{
	struct decoder_struct dec;
	uint16_t addr;
	uint8_t byte;
	uint32_t timeout;
	char c;

    addr = dest_addr;
    init_decoder(&dec, &load_data_write, &addr);

	if (!quiet)
	{
		strcpy_P((char *)buffer, text_press);
		put_string((const char *)buffer);

		keyboard_flush();
		c = keyboard_asc();
		while (c != 0x0a && c!= 0x1b)
		{
			video_wait();
			keyboard_poll();
			c = keyboard_asc();
		}

		if (c == 0x1b)
			return 1;
	}

	video_wait();
	video_off();

	// enable audio in
	ain_on();

	timeout = LOAD_TIMEOUT;
	while(1)
	{
		while (!ain_ready() && timeout)
			timeout--;

		if (!timeout)
		{
			if (!quiet)
			{
				strcpy_P((char *)buffer, text_err_time);
				put_string((const char *)buffer, dec.control);
			}
			break;
		}
		timeout = LOAD_TIMEOUT;

		byte = ain_get();
		if (decode(&dec, byte))
		{
			if (!quiet)
			{
				strcpy_P((char *)buffer, text_err_io);
				put_string((const char *)buffer, (dec.control * -1));
			}
			break;
		}

		if (dec.control == C_END)
		{
			if (!quiet)
			{
				strcpy_P((char *)buffer, text_bytes_ready);
				put_string((const char *)buffer, dec.length);
			}
			break;
		}
	}

	// disable audio in
	ain_off();

	video_on();

	return (dec.control == C_END ? 0 : dec.control);
}

void
save_queue(int16_t data, void *param)
{
	ain_put((int8_t)(data >> 8) + 128);
}

uint8_t
save(uint16_t start_addr, uint16_t end_addr, uint8_t quiet)
{
	uint16_t data_len = end_addr - start_addr, i;
	uint8_t byte, err = 0;
	struct encoder_struct enc;
	char c;
	
	enc.write = save_queue;
	enc.param = NULL; // not used
	enc.volume = 32000;

	if (!quiet)
	{
		strcpy_P((char *)buffer, text_press);
		put_string((const char *)buffer);

		keyboard_flush();
		c = keyboard_asc();
		while (c != 0x0a && c!= 0x1b)
		{
			video_wait();
			keyboard_poll();
			c = keyboard_asc();
		}

		if (c == 0x1b)
			return 1;
	}

	video_off();
	aout_on();

	encode_header(&enc, data_len);

	for (i = 0; i < data_len && !aout_err(); i++)
	{
		sram_read(start_addr++, &byte, 1);
		encode_byte(&enc, byte);
	}

	// don't encode the end if there was an error
	if (!aout_err())
		encode_end(&enc);

	// save the error state
	err = aout_err();

	// wait until the buffer is empty
	while (!aout_err());

	aout_off();
	video_on();

	if (err)
	{
		if (!quiet)
		{
			strcpy_P((char *)buffer, text_err_io);
			put_string((const char *)buffer, 0xff);
		}

		return 1;
	}

	if (!quiet)
	{
		strcpy_P((char *)buffer, text_bytes_ready);
		put_string((const char *)buffer, data_len);
	}

	return 0;
}

int
get_param(uint8_t *buffer, uint16_t *addr)
{
	uint8_t pt = 0;

	// eat whitespace
	while (buffer[pt] == ' ')
		pt++;

	if (!buffer[pt])
		return 0;

	if(sscanf((const char *)buffer + pt, "%x", addr) != 1)
		return -1;

	// calculate the end position of the parameter
	for (; buffer[pt] && isxdigit(buffer[pt]); pt++);

	return pt;
}

void
cmd_save()
{
	uint16_t start_addr = 0x1a00, end_addr = 0xffff;
	uint8_t next_param;

	if (buffer[4] != 0 && buffer[4] != ' ')
	{
		strcpy_P((char *)buffer, text_err_cmd);
		put_string((const char *)buffer);
		return;
	}

	if ((next_param = get_param(buffer + 4, &start_addr)) < 0)
	{
		strcpy_P((char *)buffer, text_err_addr);
		put_string((const char *)buffer);
		return;
	}

	next_param += 4;

	if (buffer[next_param] != 0 && buffer[next_param] != ' ')
	{
		strcpy_P((char *)buffer, text_err_cmd);
		put_string((const char *)buffer);
		return;
	}

	if (get_param(buffer + next_param, &end_addr) < 0)
	{
		strcpy_P((char *)buffer, text_err_addr);
		put_string((const char *)buffer);
		return;
	}

	if (start_addr >= end_addr)
	{
		strcpy_P((char *)buffer, text_err_addr);
		put_string((const char *)buffer);
		return;
	}

	// quiet = 0; show errors on screen
	save(start_addr, end_addr, 0);
}

void
cmd_load()
{
	// quiet = 0; show errors on screen
	load(PROG_START, 0);
}

void
cmd_run()
{
	vm_init();
	prog_exit = 0;
	while (!prog_exit && vm_exec());
}

void
cmd_cls()
{
	video_cls(' ');
	x = y = 0;
}

void
cmd_peek()
{
	static uint16_t addr = 0;
	uint8_t i, j;

	if (buffer[4] != 0 && buffer[4] != ' ')
	{
		strcpy_P((char *)buffer, text_err_cmd);
		put_string((const char *)buffer);
		return;
	}

	if (get_param(buffer + 4, &addr) < 0)
	{
		strcpy_P((char *)buffer, text_err_addr);
		put_string((const char *)buffer);
	}
	else
	{
		for (j = 0; j < 8; j++)
		{
			sram_read(addr, buffer, 6);

			put_string(" %04x:", addr);
			for(i = 0; i < 6; i++)
				put_string(" %02x", buffer[i]);

			put_string(" ");

			for(i = 0; i < 6; i++)
				put_string("%c", (buffer[i] == 0x09 || buffer[i] == 0x0a) ? '.' : buffer[i]);

			put_string("\n");

			addr += 6;
		}
	}
}

void
cmd_poke()
{
	static uint16_t addr = 0;
	uint8_t pt = 0, v;
	uint16_t v16;

	if (buffer[4] != 0 && buffer[4] != ' ')
	{
		strcpy_P((char *)buffer, text_err_cmd);
		put_string((const char *)buffer);
		return;
	}

	if (get_param(buffer + 4, &addr) < 0)
	{
		strcpy_P((char *)buffer, text_err_addr);
		put_string((const char *)buffer);
	}
	else
	{
		put_string(" %04x: ", addr);
		keyboard_flush();
		if (!buffered_input(buffer, 3))
			return;

		if(sscanf((const char *)buffer + pt, "%x", &v16) != 1)
			return;

		v = v16 & 0xff;
		sram_write(addr, &v, 1);
		addr++;
	}
}

void
cmd_list()
{
	static uint16_t addr = PROG_START;
	uint8_t i, j;
	char out[16];
	uint8_t op[3], op_len;

	if (buffer[4] != 0 && buffer[4] != ' ')
	{
		strcpy_P((char *)buffer, text_err_cmd);
		put_string((const char *)buffer);
		return;
	}

	if (get_param(buffer + 4, &addr) < 0)
	{
		strcpy_P((char *)buffer, text_err_addr);
		put_string((const char *)buffer);
	}
	else
	{
		for (j = 0; j < 12; j++)
		{
			put_string(" %04x: ", addr);
			sram_read(addr, op, 3);
			op_len = dasm_das(addr, op, out);
			for (i = 0; i < 3; i++)
				if (i < op_len)
					put_string("%02x", op[i]);
				else
					put_string("  ");
			put_string("   %s\n", out);
			addr += op_len;
		}
	}
}

void
cmd_as()
{
	static uint16_t addr = PROG_START;
	uint8_t *op = buffer + 64;
	int op_len;

	if (buffer[2] != 0 && buffer[2] != ' ')
	{
		strcpy_P((char *)buffer, text_err_cmd);
		put_string((const char *)buffer);
		return;
	}

	if (get_param(buffer + 2, &addr) < 0)
	{
		strcpy_P((char *)buffer, text_err_addr);
		put_string((const char *)buffer);
	}
	else
	{
		buffer[0] = 0;
		put_string(" %04x: ", addr);
		keyboard_flush();
		if (!buffered_input(buffer, 32))
			return;

		op_len = dasm_as(addr, (char *)buffer, op);
		if (op_len > 0)
		{
			sram_write(addr, op, op_len);
			addr += op_len;

			strcpy_P((char *)buffer, text_err_ok);
			put_string((const char *)buffer);
		}
		else
		{
			strcpy_P((char *)buffer, text_err_syntax);
			put_string((const char *)buffer);
		}
	}
}

void
cmd_help()
{
	strcpy_P((char *)buffer, text_cmd_help);
	put_string((const char *)buffer);
}

#define CMD_COUNT 9
const Cmd cmds[] PROGMEM = {
	{ text_run, cmd_run, 0 },
	{ text_cls, cmd_cls, 0 },
	{ text_peek, cmd_peek, 1 },
	{ text_poke, cmd_poke, 1 },
	{ text_load, cmd_load, 0 },
	{ text_save, cmd_save, 1 },
	{ text_list, cmd_list, 1 },
	{ text_as, cmd_as, 1 },
	{ text_help, cmd_help, 0 }
};

int
main()
{
	uint8_t i;
	int r;
	void (*cmd)();

	cli();
	video_init();
	keyboard_init();
	sei();

	video_cls(' ');

	strcpy_P((char *)buffer, text_welcome);
	put_string((const char *)buffer, MAX_RAM - PROG_START);

	while (1)
	{
		i = buffered_input(buffer, 64);
		if (!i)
			continue;

		for(i = 0; i < CMD_COUNT; i++)
		{
			if (pgm_read_byte(&cmds[i].has_params))
				r = strncasecmp_P((const char *)buffer, (const char *)pgm_read_word(&cmds[i].text), strlen_P((const char *)pgm_read_word(&cmds[i].text)));
			else
				r = strcasecmp_P((const char *)buffer, (const char *)pgm_read_word(&cmds[i].text));
			if (!r)
			{
				cmd = (void *)pgm_read_word(&cmds[i].exec);
				cmd();
				break;
			}
		}

		if (i == CMD_COUNT)
		{
			strcpy_P((char *)buffer, text_err_cmd);
			put_string((const char *)buffer);
		}
	}
}

