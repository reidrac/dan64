/*
 * init.h
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
#ifndef _INIT_H
#define _INIT_H

#include <stdint.h>

typedef struct struct_cmd {
	const char *text;
	void (*exec)();
	uint8_t has_params;
} Cmd;

void scroll_up();
uint8_t buffered_input(uint8_t *buffer, uint8_t size);
void put_char(char c);
void put_string(const char *fmt, ...);
uint8_t load(uint16_t dest_addr, uint8_t quiet);
uint8_t save(uint16_t start_addr, uint16_t end_addr, uint8_t quiet);

void vm_ram_read(uint16_t addr, uint8_t *dst, uint8_t size);
void vm_ram_write(uint16_t addr, uint8_t *src, uint8_t size);
void vm_syscall(uint8_t func);

void cmd_load();
void cmd_run();
void cmd_cls();
void cmd_peek();
void cmd_poke();
void cmd_help();

#endif // _INIT_H

