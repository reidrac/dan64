/*
 * storage.c
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

#include <stdint.h>
#include <string.h>

#include "storage.h"

#ifdef AVR
#include "hardware.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

volatile uint8_t ain_buffer[AIN_BUFFER_SIZE];
volatile uint8_t ain_start, ain_end;
volatile uint8_t ain_sync;
volatile uint8_t _aout_err;

uint8_t
aout_err()
{
	return _aout_err;
}

uint8_t
ain_full()
{
	return ((ain_end + 1) % AIN_BUFFER_SIZE == ain_start);
}

uint8_t
ain_ready()
{
	return (ain_end != ain_start);
}

uint8_t
ain_get()
{
	uint8_t byte = 0;

	if (ain_ready())
	{
		ATOMIC_BLOCK(ATOMIC_FORCEON)
		{
			byte = ain_buffer[ain_start];
			ain_start = (ain_start + 1) % AIN_BUFFER_SIZE;
		}
	}

	return byte;
}

void
ain_put(uint8_t byte)
{
	// wait until there's space in the buffer
	while (ain_full());

	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		ain_buffer[ain_end] = byte;
		ain_end = (ain_end + 1) % AIN_BUFFER_SIZE;
		if (ain_end == ain_start)
			ain_start = (ain_start + 1) % AIN_BUFFER_SIZE;
	}
}

void
ain_on()
{
	// flush buffer
	ain_start = 0;
	ain_end = 0;

	// wait for sync
	ain_sync = 0;

	cli();
	// Audio in (Arduino analog 0)
	DDRC &= ~_BV(DDC0);
	// turn on the internal pull-up resistor
	PORTC |= _BV(PORTC0);

	// Timer 2 to measure time
	// Normal mode, prescaler 256 (16MHz CPU, 62.5 KHz)
    TCCR2A = 0;
    TCCR2B = _BV(CS21) | _BV(CS22);
	TCNT2 = SHORT_TIME;

	// enable external interrupts for PCINT8
	PCICR = _BV(PCIE1);
	PCMSK1 |= _BV(PCINT8);
	sei();
}

void
ain_off()
{
	cli();
	// disable pin change interrups
	PCICR = 0;
	PCMSK1 = 0;

	// stop the clock
    TCCR2B = 0;

	// clear the overflow flag
	TIFR2 |= _BV(TOV2);

	// turn off the internal pull-up resistor
	PORTC &= ~_BV(PORTC0);
	sei();
}

void
aout_on()
{
	cli();
	// flush buffer
	ain_start = 0;
	ain_end = 0;

	// no errors
	_aout_err = 0;

    // set Timer 2: mode 3 (Fast PWM Mode, 8-bits)
    // no prescaler
    TCCR2A = _BV(WGM20) | _BV(WGM21) | _BV(COM2B1);
    TCCR2B = _BV(CS20);
	TCNT2 = 0;
	OCR2B = 0;
	TIFR2 = 0;

	// Audio out
	DDRD |= _BV(DDD3);

	// set timer 2 overflow interrupt
    TIMSK2 = _BV(TOIE2);
	sei();
}

void
aout_off()
{
	cli();
	// disable the interrupt
    TIMSK2 = 0;

	// disable the output pin
	DDRD &= ~_BV(DDD3);

	// stop the clock
    TCCR2B = 0;
	sei();
}

ISR (TIMER2_OVF_vect)
{
	static uint8_t cnt = 4;

	if (cnt++ < 4)
		return;
	cnt = 0;

	// empty buffer, something bad happened
	if (ain_end == ain_start || _aout_err)
	{
		_aout_err = 1;
		OCR2B = 0;
		return;
	}

	OCR2B = ain_buffer[ain_start];
	ain_start = (ain_start + 1) % AIN_BUFFER_SIZE;
}

ISR (PCINT1_vect)
{
	static uint8_t bit = 0;
	static uint8_t byte = 0;
	uint8_t pulse_length = TCNT2;
	TCNT2 = 0;

	// discard n changes (sync)
	if (ain_sync < 4)
	{
		bit = 0;
		byte = 0;
		ain_sync++;
		return;
	}

	if (pulse_length > SHORT_TIME)
		byte |= 1 << bit;

	if (++bit == 8)
	{
		ain_buffer[ain_end] = byte;
		ain_end = (ain_end + 1) % AIN_BUFFER_SIZE;
		if (ain_end == ain_start)
			ain_start = (ain_start + 1) % AIN_BUFFER_SIZE;
		bit = 0;
		byte = 0;
	}
}

#endif // AVR

void
init_decoder(struct decoder_struct *dec, void *write_fn, void *param)
{
	dec->control = 1;
	dec->length = 0;
	dec->parity = 0;
	dec->count = 0;
	dec->write = write_fn;
	dec->param = param;
}

int8_t
decode(struct decoder_struct *dec, uint8_t byte)
{
	switch(dec->control)
	{
		case C_MAGIC:
			// magic number
			if(byte != 0xff)
			{
				fprintf(stderr, "** BAD MAGIC (%d)\n", byte);
				dec->control *= -1;
				return dec->control;
			}
			dec->control++;
			break;
		case C_LEN0:
			// length MSB
			dec->length = byte;
			dec->parity ^= byte;
			dec->control++;
			break;
		case C_LEN1:
			// length LSB
			dec->length = (dec->length << 8) | byte;
			dec->parity ^= byte;
			dec->control++;
			break;
		case C_PAR0:
			// parity check
			if (byte != dec->parity)
			{
				fprintf(stderr, "** HEADER PARITY ERROR (%i, expected %i)\n", byte, dec->parity);
				dec->control *= -1;
				return dec->control;
			}
			dec->parity = 0;
			dec->control++;
			break;
		case C_DATA:
			// data
			dec->count++;
			dec->parity ^= byte;
			dec->write(byte, dec->param);
			if (dec->count == dec->length)
				dec->control++;
			break;
		case C_PAR1:
			// parity check
			if (byte != dec->parity)
			{
				fprintf(stderr, "** DATA PARITY ERROR (%i, expected %i)\n", byte, dec->parity);
				dec->control *= -1;
				return dec->control;
			}
			dec->control++;
			break;
		case C_END:
			return 0;
		default:
			fprintf(stderr, "** UNEXPECTED ERROR\n");
			dec->control *= -1;
			return dec->control;
	}

	return 0;
}

static const uint16_t _freqs[] = {
	SAMPLERATE/LONG_PULSE,
	SAMPLERATE/SHORT_PULSE
};

void
half_pulse(struct encoder_struct *enc, uint8_t bit)
{
	uint16_t i;
	int16_t sample;

	for (i = 0; i < _freqs[bit]; i++)
	{
		sample = enc->volume * enc->level;
		enc->write(sample, enc->param);
	}
	enc->level = enc->level > 0 ? -1 : 1;
}

static void
_encode_byte(struct encoder_struct *enc, uint8_t byte)
{
	uint8_t bit;

	for (bit = 0; bit < 8; bit++)
		half_pulse(enc, !((byte >> bit) & 1));
}

void
encode_byte(struct encoder_struct *enc, uint8_t byte)
{
	_encode_byte(enc, byte);
	enc->parity ^= byte;
}

void
encode_header(struct encoder_struct *enc, uint16_t length)
{
	enc->parity = 0;

#if AVR
	enc->level = -1;
#else
	enc->level = 1;
	half_pulse(enc, 1);
#endif // AVR

	// sync
	half_pulse(enc, 1);

	// magic
	_encode_byte(enc, 0xff);

	// length (word, MSB first)
	encode_byte(enc, (uint8_t)(length >> 8));
	encode_byte(enc, (uint8_t)(length & 0xff));

	// header parity (not including magic)
	_encode_byte(enc, enc->parity);

	enc->parity = 0;
}

void
encode_end(struct encoder_struct *enc)
{
	_encode_byte(enc, enc->parity);

	half_pulse(enc, 0);
	enc->level = -1;
	half_pulse(enc, 0);
}

