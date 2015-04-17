/*
 * video.h
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

#ifndef _VIDEO_H
#define _VIDEO_H

#include <stdint.h>

#define VIDEO_ADDR				0x0200

#define PAL_CYCLES_SCANLINE		((64 * F_CPU / 1000000) - 1)

#define CHARS_WIDTH				32
#define CHARS_HEIGHT			24

#define PAL_LINES_CHARS			(CHARS_HEIGHT * 8)

#define PAL_LINES_PER_FRAME		312
#define PAL_LINES_DBEGIN		((PAL_LINES_PER_FRAME / 2) - (PAL_LINES_CHARS / 2) + 8)
#define PAL_LINES_DEND			(PAL_LINES_DBEGIN + PAL_LINES_CHARS)

#define wait_spi_done()			loop_until_bit_is_set(SPSR, SPIF)

void video_init();
void video_wait();
void video_off();
void video_on();
void video_cls(uint8_t c);
void video_put_char(uint8_t x, uint8_t y, uint8_t c);
void video_cursor(uint8_t x, uint8_t y);
void video_cursor_off(uint8_t x, uint8_t y);
void video_put_tile(uint8_t x, uint8_t y, const uint8_t *tile);

#endif // _VIDEO_H

