/*
 * dasm.h
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
#ifndef _DASM_H
#define _DASM_H

#ifdef AVR
#include "hardware.h"
#include <avr/pgmspace.h>
#else // not AVR
#define PROGMEM /* */
#define memcpy_P		memcpy
#define memcmp_P		memcmp
#define pgm_read_byte(x) (*x)
#endif // AVR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

// addressing types
enum addr_enum {
	AT_IMPLIED = 0,
	AT_ABSOLUTE,
	AT_IMMEDIATE,
	AT_ZEROP,
	AT_IND_ABS,
	AT_ABS_INDEX_X,
	AT_ABS_INDEX_Y,
	AT_ZP_INDEX_X,
	AT_ZP_INDEX_Y,
	AT_INDEX_IND,
	AT_IND_INDEX,
	AT_RELATIVE,
	AT_ACCUMULATOR
};

uint8_t dasm_das(uint16_t addr, uint8_t *op, char *output);
int dasm_as(uint16_t addr, char *input, uint8_t *output);

#endif // _DASM_H

