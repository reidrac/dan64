/*
 * vm.h
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

#ifndef _VM_H
#define _VM_H

#include <stdint.h>

#define addr16(x, y)		((x) | ((y) << 8))
#define sbit(x)				(1 << x)
#define testZ(x)			((x) ?  0 : sbit(Zf))
#define testN(x)            ((x) & sbit(Nf))
#define testC(x)            ((x) & sbit(Cf))
#define testV(x)			((x) & sbit(Vf))
#define testD(x)			((x) & sbit(Df))

#define Cf					0
#define Zf					1
#define If					2
#define Df					3
#define Bf					4
#define Vf					6
#define Nf					7

#define PROG_START			0x1a00

void vm_init();
uint8_t vm_exec();

#endif // _VM_H

