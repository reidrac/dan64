/*
 * video.c
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
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>

#include "memory.h"
#include "video.h"
#include "font.h"

volatile uint8_t vsync;
volatile uint16_t scanline;
volatile uint8_t adj_pal_lines;
volatile uint8_t cursor = 0;

void
video_wait()
{
	while(!vsync);
}

void
video_init()
{
	// SRAM CS, DATA sel0, DATA sel1
	DDRD |= _BV(DDD6) | _BV(DDD2) | _BV(DDD5);
	// de-selected RAM
	PORTD |= _BV(PORTD6);
	// disable DATA
	PORTD |= _BV(PORTD2);
	PORTD &= ~_BV(PORTD5);

	// PAL SYNC port (Arduino pin 7)
	DDRD |= _BV(DDD7);

	DDRB |= _BV(DDB3) | _BV(DDB5) | _BV(DDB2); // MOSI, SCK, SS
	SPSR = _BV(SPI2X);
	SPCR = _BV(SPE) | _BV(MSTR) | _BV(CPOL) | _BV(CPHA);

	// Timer 1, fast PWM no prescaler
	TCCR1A = _BV(WGM11);
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);

	TCNT1 = 0;
	ICR1 = PAL_CYCLES_SCANLINE;

	video_on();
}

void
video_off()
{
    TIMSK1 &= ~_BV(TOIE1);
	vsync = 1;
}

void
video_on()
{
	adj_pal_lines = 1;
	vsync = 0;
	scanline = 310;

	// set the timer overflow interrupt
    TIMSK1 = _BV(TOIE1);
}

void
video_cls(uint8_t c)
{
	uint8_t y, line;
	uint16_t addr, pt = c * 8;

	video_wait();

	for (y = 0; y < CHARS_HEIGHT; y++)
		for (line = 0; line < 8; line++)
		{
			addr = VIDEO_ADDR + ((y * CHARS_WIDTH) * 8) + (line * CHARS_WIDTH);
			c = pgm_read_byte(&font[line + pt]);
			sram_set(addr, c, CHARS_WIDTH);
		}
}

void
video_put_char(uint8_t x, uint8_t y, uint8_t c)
{
	uint16_t addr = VIDEO_ADDR + (x + y * CHARS_WIDTH * 8), c_start = (c << 3);
	uint8_t line;

	for (line = 0; line < 8; line++)
	{
		c = pgm_read_byte(&font[line + c_start]);
		sram_write(addr, &c, 1);
		addr += CHARS_WIDTH;
	}
}

void
video_cursor(uint8_t x, uint8_t y)
{
	uint16_t addr = VIDEO_ADDR + (x + y * CHARS_WIDTH * 8);
	uint8_t c, line;

	for (line = 0; line < 8; line++)
	{
		sram_read(addr, &c, 1);
		c = ~c;
		sram_write(addr, &c, 1);

		addr += CHARS_WIDTH;
	}

	cursor = !cursor;
}

void
video_cursor_off(uint8_t x, uint8_t y)
{
	if (cursor)
		video_cursor(x, y);
}

void
video_put_tile(uint8_t x, uint8_t y, const uint8_t *tile)
{
	uint16_t addr = VIDEO_ADDR + (x + y * CHARS_WIDTH * 8);
	uint8_t line;

	for (line = 0; line < 8; line++)
	{
		sram_write(addr, &tile[line], 1);
		addr += CHARS_WIDTH;
	}
}

ISR(TIMER1_OVF_vect)
{
	uint8_t column = CHARS_WIDTH;
	uint16_t addr;

	if (scanline > 5 && scanline < 310)
	{
		// horizontal sync
		PORTD &= ~_BV(PORTD7);
		_delay_us(5.25);
		PORTD |= _BV(PORTD7);
		_delay_us(7.0);
	}
	else
	{
		// extra delay to avoid first line artifacts
		_delay_us(0.8);

		// vertical sync
		PORTD |= _BV(PORTD7);
		_delay_us(10);
		PORTD &= ~_BV(PORTD7);

		if (scanline == 310)
			vsync = 1;
	}

	// the RAM needs extra time :(
	if (scanline + 6 == PAL_LINES_DBEGIN)
		vsync = 0;

	if (scanline >= PAL_LINES_DBEGIN && scanline < PAL_LINES_DEND)
	{
		// select SRAM
		PORTD &= ~_BV(PORTD6);

		// READ to make SRAM to dump video RAM
		SPDR = 0x03;
		wait_spi_done();

		addr = VIDEO_ADDR + (scanline - PAL_LINES_DBEGIN) * CHARS_WIDTH;
		// addr for video RAM
		SPDR = (uint8_t)(addr >> 8);
		wait_spi_done();
		SPDR = (uint8_t)(addr);
		wait_spi_done();

		do
		{
			SPDR = 0xff;
			_delay_us(0.752);
		}
		while(--column);

		// deselect SRAM
		PORTD |= _BV(PORTD6);

		wait_spi_done();
	}

	if (scanline == 6)
	{
		// de-interlace adjustment
		adj_pal_lines = adj_pal_lines ? 0 : 1;
		scanline -= adj_pal_lines;
	}

	scanline++;
	if (scanline > PAL_LINES_PER_FRAME)
		scanline = 1;
}

