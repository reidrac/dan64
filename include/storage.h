/*
 * storage.h
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

#ifndef _STORAGE_H
#define _STORAGE_H

#ifdef AVR

#define fprintf(...) /* ... */

#define AIN_BUFFER_SIZE	32
#define SAMPLERATE		15625

void ain_on();
void ain_off();
uint8_t ain_ready();
uint8_t ain_get();
void ain_put(uint8_t byte);
uint8_t ain_full();

void aout_on();
void aout_off();
uint8_t aout_err();

#else // not AVR

#include <stdio.h>
#define SAMPLERATE		44100

#endif // AVR

#define MAGIC			0xff

#define LONG_PULSE		1200
#define SHORT_PULSE		(LONG_PULSE * 2.3)

#define LOAD_TIMEOUT	0x1fffff;

// based on 256 prescaler for timer 2
#define SHORT_TIME		(((F_CPU / 256) / SHORT_PULSE) + 16)

#define C_MAGIC			1
#define C_LEN0			2
#define C_LEN1			3
#define C_PAR0			4
#define C_DATA			5
#define C_PAR1			6
#define C_END			7

struct decoder_struct
{
	int8_t	control;
	uint16_t length;
	uint8_t	parity;
	uint16_t count;

	void (*write)(uint8_t, void *);
	void *param;
};

struct encoder_struct
{
	int16_t volume;
	uint8_t parity;
	int8_t level;

	void (*write)(int16_t, void *);
	void *param;
};

void init_decoder(struct decoder_struct *dec, void *write_fn, void *param);
int8_t decode(struct decoder_struct *dec, uint8_t bit);

void encode_byte(struct encoder_struct *enc, uint8_t byte);
void encode_header(struct encoder_struct *enc, uint16_t lenght);
void encode_end(struct encoder_struct *enc);

#endif // _STORAGE_H

