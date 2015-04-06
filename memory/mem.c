/*
 * mem.c
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
#include <stdint.h>

#include "video.h"
#include "memory.h"

void
sram_write(uint16_t addr, const uint8_t *data, uint16_t size)
{
	uint16_t pt;

	video_wait();

	// select SRAM
	PORTD &= ~_BV(PORTD6);

	// WRITE to SRAM
	SPDR = 0x02;
	wait_spi_done();

	SPDR = (uint8_t)(addr >> 8);
	wait_spi_done();
	SPDR = (uint8_t)(addr);
	wait_spi_done();

	for (pt = 0; pt < size; pt++)
	{
		SPDR = data[pt];
		wait_spi_done();
	}

	// deselect SRAM
	PORTD |= _BV(PORTD6);
}

void
sram_set(uint16_t addr, uint8_t c, uint16_t times)
{
	uint16_t i;

	video_wait();

	// select SRAM
	PORTD &= ~_BV(PORTD6);

	// WRITE to SRAM
	SPDR = 0x02;
	wait_spi_done();

	SPDR = (uint8_t)(addr >> 8);
	wait_spi_done();
	SPDR = (uint8_t)(addr);
	wait_spi_done();

	for (i = 0; i < times; i++)
	{
		SPDR = c;
		wait_spi_done();
	}

	// deselect SRAM
	PORTD |= _BV(PORTD6);
}

void
sram_read(uint16_t addr, uint8_t *data, uint16_t size)
{
	uint16_t pt;

	video_wait();

	// enable DATA
	PORTD &= ~_BV(PORTD2);
	PORTD |= _BV(PORTD5);

	// select SRAM
	PORTD &= ~_BV(PORTD6);

	// READ from SRAM
	SPDR = 0x03;
	wait_spi_done();

	SPDR = (uint8_t)(addr >> 8);
	wait_spi_done();
	SPDR = (uint8_t)(addr);
	wait_spi_done();

	for (pt = 0; pt < size; pt++)
	{
		SPDR = 0xff;
		wait_spi_done();
		data[pt] = SPDR;
	}

	// deselect SRAM
	PORTD |= _BV(PORTD6);

	// disable DATA
	PORTD |= _BV(PORTD2);
	PORTD &= ~_BV(PORTD5);
}

