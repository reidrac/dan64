/*
 * keyboard.c (PS/2 driver using USART)
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
#include <avr/pgmspace.h>
#include <stdint.h>

#include "keyboard.h"
#include "keyb_map.h"

volatile uint8_t keyb_buffer[KEYB_BUFFER_SIZE];
volatile uint8_t keyb_start = 0, keyb_end = 0;
volatile uint8_t keyb_shift = 0, keyb_released = 0;

void
keyboard_flush()
{
	keyb_start = keyb_end = 0;
}

void
keyboard_poll()
{
	if (UCSR0A & _BV(RXC0))
	{
		keyb_buffer[keyb_end] = UDR0;
		keyb_end = (keyb_end + 1) % KEYB_BUFFER_SIZE;
		if (keyb_end == keyb_start)
			keyb_start = (keyb_start + 1) % KEYB_BUFFER_SIZE;
	}
}

uint8_t
keyboard_scancode()
{
	uint8_t sc = 0;

	if (keyb_start != keyb_end)
	{
		sc = keyb_buffer[keyb_start];
		keyb_start = (keyb_start + 1) % KEYB_BUFFER_SIZE;
	}

	return sc;
}

uint8_t
keyboard_asc()
{
	uint8_t sc;

    while (1)
    {
		keyboard_poll();
        sc = keyboard_scancode();
        if (!sc)
            // error or empty buffer
            return 0;

        switch(sc)
        {
            case 0xf0:
                keyb_released = 1;
                break;
            case 0x12:
            case 0x59:
                keyb_shift = !keyb_released;
                keyb_released = 0;
                break;
            default:
                if (keyb_released)
                    sc = 0;
                keyb_released = 0;
                if (!sc || sc > KEYB_MAX_SCANCODE)
                    return 0;
                return pgm_read_byte(&keyb_map[sc][keyb_shift]);
        }
    }

    return 0;
}

void
keyboard_init()
{
	// PD4 is XCK
	// PD0 is RX

	// setup USART

	UCSR0A = 0;

	UBRR0H = KEYB_BAUDRATE >> 8;
	UBRR0L = KEYB_BAUDRATE;

	// only receiver and sender
	UCSR0B = _BV(RXEN0);
	// synchronous, odd parity, 8-bit character size (default)
	UCSR0C |= _BV(UMSEL00) | _BV(UPM00) | _BV(UPM01);
}

