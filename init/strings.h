/*
 * string.h
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

#ifdef _INIT_C
const char text_welcome[] PROGMEM = "DAN64 v1, %u bytes free\nReady\n";

const char text_run[] PROGMEM = "run";
const char text_cls[] PROGMEM = "cls";
const char text_peek[] PROGMEM = "peek";
const char text_poke[] PROGMEM = "poke";
const char text_load[] PROGMEM = "load";
const char text_save[] PROGMEM = "save";
const char text_help[] PROGMEM = "help";
const char text_list[] PROGMEM = "list";
const char text_as[] PROGMEM = "as";

const char text_bytes_ready[] PROGMEM = "%i bytes\nReady\n";
const char text_press[] PROGMEM = "Press <ENTER> when ready,\n<ESC> to cancel...\n";

const char text_cmd_help[] PROGMEM = " LOAD, SAVE [addr [addr]], RUN\n LIST [addr], AS [addr]\n PEEK [addr], POKE [addr],\n CLS, HELP\n";

const char text_err_ok[] PROGMEM = "Ok\n";
const char text_err_addr[] PROGMEM = "ERR: ADDR\n";
const char text_err_cmd[] PROGMEM = "ERR: CMD\n";
const char text_err_sys[] PROGMEM = "ERR: SYS %02x\n";
const char text_err_prg[] PROGMEM = "ERR: PRG %02x\n";
const char text_err_time[] PROGMEM = "ERR: TIME %02x\n";
const char text_err_io[] PROGMEM = "ERR: IO %02x\n";
const char text_err_syntax[] PROGMEM = "ERR: SYNTAX\n";

#else // _INIT_C
#ifndef _INIT_STRINGS_H
#define _INIT_STRINGS_H

extern const char text_welcome[] PROGMEM;

extern const char text_run[] PROGMEM;
extern const char text_cls[] PROGMEM;
extern const char text_peek[] PROGMEM;
extern const char text_poke[] PROGMEM;
extern const char text_load[] PROGMEM;
extern const char text_save[] PROGMEM;
extern const char text_help[] PROGMEM;

extern const char text_bytes_ready[] PROGMEM;
extern const char text_press[] PROGMEM;

extern const char text_cmd_help[] PROGMEM;

extern const char text_err_ok[] PROGMEM;
extern const char text_err_addr[] PROGMEM;
extern const char text_err_cmd[] PROGMEM;
extern const char text_err_sys[] PROGMEM;
extern const char text_err_prg[] PROGMEM;
extern const char text_err_time[] PROGMEM;
extern const char text_err_io[] PROGMEM;
extern const char text_err_syntax[] PROGMEM;
#endif // _INIT_STRINGS_h
#endif // _INIT_C

