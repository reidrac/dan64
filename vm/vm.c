/*
 * vm.c (6502 virtual machine)
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

#include "vm.h"

// externally defined
void vm_ram_read(uint16_t addr, uint8_t *dst, uint8_t size);
void vm_ram_write(uint16_t addr, uint8_t *src, uint8_t size);
void vm_syscall(uint8_t func);

uint8_t r_a, r_x, r_y, r_sp, r_s;
uint16_t r_pc;

uint8_t op, r_t, r_ts;
uint16_t addr, r_t16;

static uint8_t _op_cache[5];

void
vm_init()
{
	r_a = r_x = r_y = r_s = 0;
	r_sp = 0xff;
	r_pc = PROG_START;
}

uint8_t
vm_exec()
{
	uint8_t *pt = _op_cache;

	vm_ram_read(r_pc, pt, 3);
	op = *pt++;

	switch(op)
	{
		case 0x02:
			// SYS
			vm_syscall(r_a);
			break;
		case 0xca:
			// DEX
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_x--;
			r_s |=  testN(r_x) | testZ(r_x);;
			break;

		case 0x88:
			// DEY
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_y--;
			r_s |= testN(r_y) | testZ(r_y);
			break;

		case 0xaa:
			// TAX
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_x = r_a;
			r_s |= testN(r_x) | testZ(r_x);
			break;

		case 0x10:
			// BPL
			if (!testN(r_s))
				r_pc += (int8_t)*pt++;
			else
				r_pc++;
			break;

		case 0x90:
			// BCC
			if (!(r_s & sbit(Cf)))
				r_pc += (int8_t)*pt++;
			else
				r_pc++;
			break;

		case 0xe0:
			// CPX #n
			r_t = *pt++;
			goto cpx_im;
		case 0xe4:
			// CPX zp
			addr = *pt++;
			goto cpx;
		case 0xec:
			// CPX abs
			addr = addr16(*pt++, *pt++);
cpx:
			vm_ram_read(addr, &r_t, 1);
cpx_im:
			r_ts = r_x - r_t;
			r_s &= ~(sbit(Nf) | sbit(Zf) | sbit(Cf));
			r_s |= testN(r_ts) | testZ(r_ts) | (r_x >= r_t ? sbit(Cf) : 0);
			break;

		case 0x41:
			// EOR (zp,x)
			addr = ((*pt++) + r_x) & 0xff;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]);
			goto eor;
		case 0x45:
			// EOR zp
			addr = *pt++;
			goto eor;
		case 0x49:
			// EOR #n
			r_a ^= *pt++;
			goto eor_im;
		case 0x4d:
			// EOR abs
			addr = addr16(*pt++, *pt++);
			goto eor;
		case 0x51:
			// EOR (zp),y
			addr = *pt++;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]) + r_y;
			goto eor;
		case 0x55:
			// EOR zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto eor;
		case 0x59:
			// EOR abs,y
			addr = addr16(*pt++, *pt++) + r_y;
			goto eor;
		case 0x5d:
			// EOR abs,x
			addr = addr16(*pt++, *pt++) + r_x;
eor:
			vm_ram_read(addr, &r_t, 1);
			r_a ^= r_t;
eor_im:
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_s |= testN(r_a) | testZ(r_a);
			break;

		case 0xba:
			// TSX
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_x = r_sp;
			r_s |= testN(r_x) | testZ(r_x);
			break;

		case 0xc6:
			// DEC zp
			addr = *pt++;
			goto dec;
		case 0xce:
			// DEC abs
			addr = addr16(*pt++, *pt++);
			goto dec;
		case 0xd6:
			// DEC zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto dec;
		case 0xde:
			// DEC abs,x
			addr = addr16(*pt++, *pt++) + r_x;
dec:
			vm_ram_read(addr, &r_t, 1);
			r_t--;
			vm_ram_write(addr, &r_t, 1);
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_s |= testN(r_t) | testZ(r_t);
			break;

		case 0x81:
			// STA (zp,x)
			addr = ((*pt++) + r_x) & 0xff;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]);
			vm_ram_write(addr, &r_a, 1);
			break;
		case 0x85:
			// STA zp
			addr = *pt++;;
			vm_ram_write(addr, &r_a, 1);
			break;
		case 0x8d:
			// STA abs
			addr = addr16(*pt++, *pt++);
			vm_ram_write(addr, &r_a, 1);
			break;
		case 0x91:
			// STA (zp),y
			addr = *pt++;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]) + r_y;
			vm_ram_write(addr, &r_a, 1);
			break;
		case 0x95:
			// STA zp,x
			addr = ((*pt++) + r_x) & 0xff;
			vm_ram_write(addr, &r_a, 1);
			break;
		case 0x99:
			// STA abs,y
			addr = addr16(*pt++, *pt++) + r_y;
			vm_ram_write(addr, &r_a, 1);
			break;
		case 0x9d:
			// STA abs,x
			addr = addr16(*pt++, *pt++) + r_x;
			vm_ram_write(addr, &r_a, 1);
			break;

		case 0xa1:
			// LDA (zp,x)
			addr = ((*pt++) + r_x) & 0xff;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]);
			goto lda;
		case 0xa5:
			// LDA zp
			addr = *pt++;
			goto lda;
		case 0xa9:
			// LDA #n
			r_a = *pt++;
			goto lda_im;
		case 0xad:
			// LDA abs
			addr = addr16(*pt++, *pt++);
			goto lda;
		case 0xb1:
			// LDA (zp),y
			addr = *pt++;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]) + r_y;
			goto lda;
		case 0xb5:
			// LDA zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto lda;
		case 0xb9:
			// LDA abs,y
			addr = addr16(*pt++, *pt++) + r_y;
			goto lda;
		case 0xbd:
			// LDA abs,x
			addr = addr16(*pt++, *pt++) + r_x;
lda:
			vm_ram_read(addr, &r_a, 1);
lda_im:
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_s |= testN(r_a) | testZ(r_a);
			break;

		case 0xf0:
			// BEQ
			if (r_s & sbit(Zf))
				r_pc += (int8_t)*pt++;
			else
				r_pc++;
			break;

		case 0x26:
			// ROL zp
			addr = *pt++;
			goto rol;
		case 0x2a:
			// ROL a
			r_ts = r_a;
			r_a = r_t = ((r_a << 1) & 0xfe) | testC(r_s);
			goto rol_a;
		case 0x2e:
			// ROL abs
			addr = addr16(*pt++, *pt++);
			goto rol;
		case 0x36:
			// ROL zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto rol;
		case 0x3e:
			// ROL abs,x
			addr = addr16(*pt++, *pt++) + r_x;
rol:
			vm_ram_read(addr, &r_t, 1);
			r_ts = r_t;
			r_t = ((r_t << 1) & 0xfe) | testC(r_s);
			vm_ram_write(addr, &r_t, 1);
rol_a:
			r_s &= ~(sbit(Nf) | sbit(Zf) | sbit(Cf));
			r_s |= testN(r_t) | testZ(r_t) | (testN(r_ts) ? sbit(Cf) : 0);
			break;

		case 0x84:
			// STY zp
			addr = *pt++;
			goto sty;
		case 0x8c:
			// STY abs
			addr = addr16(*pt++, *pt++);
			goto sty;
		case 0x94:
			// STY zp,x
			addr = ((*pt++) + r_x) & 0xff;
sty:
			vm_ram_write(addr, &r_y, 1);
			break;

		case 0x4c:
			// JMP abs
			r_pc = addr16(*pt++, *pt++);
			return 1;
		case 0x6c:
			// JMP (abs)
			addr = addr16(*pt++, *pt++);
			vm_ram_read(addr, pt, 2);
			r_pc = addr16(pt[0], pt[1]);
			return 1;

		case 0x30:
			// BMI
			if (r_s & sbit(Nf))
				r_pc += (int8_t)*pt++;
			else
				r_pc++;
			break;

		case 0x40:
			// RTI
			vm_ram_read(addr16(r_sp + 1, 1), pt, 3);
			r_s = pt[0];
			r_pc = addr16(pt[1], pt[2]);
			r_sp += 3;
			return 1;

		case 0xa8:
			// TAY
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_y = r_a;
			r_s |= testN(r_y) | testZ(r_y);
			break;

		case 0x8a:
			// TXA
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_a = r_x;
			r_s |= testN(r_a) | testZ(r_a);
			break;

		case 0x60:
			// RTS
			vm_ram_read(addr16(r_sp + 1, 1), pt, 2);
			r_pc = addr16(pt[0], pt[1]) + 1;
			r_sp += 2;
			return 1;

		case 0xf8:
			// SED
			r_s |= sbit(Df);
			break;

		case 0x46:
			// LSR zp
			addr = *pt++;
			goto lsr;
		case 0x4a:
			// LSR a
			r_ts = r_a;
			r_a = r_t = (r_a >> 1) & 0x7f;
			goto lsr_a;
		case 0x4e:
			// LSR abs
			addr = addr16(*pt++, *pt++);
			goto lsr;
		case 0x56:
			// LSR zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto lsr;
		case 0x5e:
			// LSR abs,x
			addr = addr16(*pt++, *pt++) + r_x;
lsr:
			vm_ram_read(addr, &r_ts, 1);
			r_t = (r_ts >> 1) & 0x7f;
			vm_ram_write(addr, &r_t, 1);
lsr_a:
			r_s &= ~(sbit(Nf) | sbit(Zf) | sbit(Cf));
			r_s |= testZ(r_t) | testC(r_ts);
			break;

		case 0x20:
			// JSR abs
			addr = r_pc + 2;
			r_pc = addr16(*pt++, *pt++);
			pt[0] = (int8_t)addr;
			pt[1] = (addr >> 8);
			vm_ram_write(addr16(r_sp - 1, 1), pt, 2);
			r_sp -= 2;
			return 1;

		case 0xa0:
			// LDY #n
			r_y = *pt++;
			goto ldy_im;
		case 0xa4:
			// LDY zp
			addr = *pt++;
			goto ldy;
		case 0xac:
			// LDY abs
			addr = addr16(*pt++, *pt++);
			goto ldy;
		case 0xb4:
			// LDY zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto ldy;
		case 0xbc:
			// LDY abs,x
			addr = addr16(*pt++, *pt++) + r_x;
ldy:
			vm_ram_read(addr, &r_y, 1);
ldy_im:
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_s |= testN(r_y) | testZ(r_y);
			break;

		case 0x38:
			// SEC
			r_s |= sbit(Cf);
			break;

		case 0x24:
		case 0x2c:
			if (op == 0x24)
				// BIT zp
				addr = *pt++;
			else
				// BIT abs
				addr = addr16(*pt++, *pt++);

			vm_ram_read(addr, &r_t, 1);

			r_s &= ~(sbit(Nf) | sbit(Vf) | sbit(Zf));
			r_s |= testN(r_t) | testV(r_t) | testZ(r_t & r_a);
			break;

		case 0xa2:
			// LDX #n
			r_x = *pt++;
			goto ldx_im;
		case 0xa6:
			// LDX zp
			addr = *pt++;
			goto ldx;
		case 0xae:
			// LDX abs
			addr = addr16(*pt++, *pt++);
			goto ldx;
		case 0xb6:
			// LDX zp,y
			addr = ((*pt++) + r_y) & 0xff;
			goto ldx;
		case 0xbe:
			// LDX abs,y
			addr = addr16(*pt++, *pt++) + r_y;
ldx:
			vm_ram_read(addr, &r_x, 1);
ldx_im:
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_s |= testN(r_x) | testZ(r_x);
			break;

		case 0x9a:
			// TXS
			r_sp = r_x;
			break;

		case 0x78:
			// SEI
			r_s |= sbit(If);
			break;

		case 0x06:
			// ASL zp
			addr = *pt++;
			goto asl;
		case 0x0a:
			// ASL a
			r_ts = r_a;
			r_a = r_t = (r_a << 1) & 0xfe;
			goto asl_a;
		case 0x0e:
			// ASL abs
			addr = addr16(*pt++, *pt++);
			goto asl;
		case 0x16:
			// ASL zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto asl;
		case 0x1e:
			// ASL abs,x
			addr = addr16(*pt++, *pt++) + r_x;
asl:
			vm_ram_read(addr, &r_t, 1);
			r_ts = r_t;
			r_t = (r_t << 1) & 0xfe;
			vm_ram_write(addr, &r_t, 1);
asl_a:
			r_s &= ~(sbit(Nf) | sbit(Zf) | sbit(Cf));
			r_s |= testN(r_t) | testZ(r_t) | (testN(r_ts) ? sbit(Cf) : 0);
			break;

		case 0x70:
			// BVS
			if (r_s & sbit(Vf))
				r_pc += (int8_t)*pt++;
			else
				r_pc++;
			break;

		case 0xc0:
			// CPY #n
			r_t = *pt++;
			goto cpy_im;
		case 0xc4:
			// CPY zp
			addr = *pt++;
			goto cpy;
		case 0xcc:
			// CPY abs
			addr = addr16(*pt++, *pt++);
cpy:
			vm_ram_read(addr, &r_t, 1);
cpy_im:
			r_ts = r_y - r_t;
			r_s &= ~(sbit(Nf) | sbit(Zf) | sbit(Cf));
			r_s |= testN(r_ts) | testZ(r_ts) | (r_y >= r_t ? sbit(Cf) : 0);
			break;

		case 0x58:
			// CLI
			r_s &= ~sbit(If);
			break;

		case 0xd8:
			// CLD
			r_s &= ~sbit(Df);
			break;

		case 0x18:
			// CLC
			r_s &= ~sbit(Cf);
			break;

		case 0xb0:
			// BCS
			if (r_s & sbit(Cf))
				r_pc += (int8_t)*pt++;
			else
				r_pc++;
			break;

		case 0x61:
			// ADC (zp,x)
			addr = ((*pt++) + r_x) & 0xff;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]);
			goto adc;
		case 0x65:
			// ADC zp
			addr = *pt++;
			goto adc;
		case 0x69:
			// ADC #n
			r_t = *pt++;

			if (testD(r_s))
				goto adc_im_d;

			goto adc_im;
		case 0x6d:
			// ADC abs
			addr = addr16(*pt++, *pt++);
			goto adc;
		case 0x71:
			// ADC (zp),y
			addr = *pt++;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]) + r_y;
			goto adc;
		case 0x75:
			// ADC zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto adc;
		case 0x79:
			// ADC abs,y
			addr = addr16(*pt++, *pt++) + r_y;
			goto adc;
		case 0x7d:
			// ADC abs,x
			addr = addr16(*pt++, *pt++) + r_x;
adc:
			vm_ram_read(addr, &r_t, 1);
adc_im_d:
			if (testD(r_s))
			{
				uint16_t t;

				r_t16 = (r_a & 0xf) + (r_t & 0xf) + testC(r_s);
				if (r_t16 > 9)
					r_t16 += 6;
				t = (r_a >> 4) + (r_t >> 4) + (r_t16 > 15 ? 1 : 0);
				if (t > 9)
					t += 6;
				r_s &= ~(sbit(Vf) | sbit(Nf) | sbit(Zf) | sbit(Cf));

				r_a = (r_t16 & 0xf) | (t << 4);
				r_s |= (t > 15 ? sbit(Cf) : 0) | testZ(r_a);
				break;
			}

adc_im:
			r_t16 = r_a + r_t + testC(r_s);
			r_s &= ~(sbit(Vf) | sbit(Nf) | sbit(Zf) | sbit(Cf));

			r_s |= (testN(~(r_a ^ r_t) & (r_a ^ (r_t16 & 0xff))) ? sbit(Vf) : 0)
				| ((r_t16 & 0xff00) ? sbit(Cf) : 0) | testN(r_t16) | testZ(r_t16 & 0xff);
			r_a = r_t16 & 0xff;
			break;

		case 0xb8:
			// CLV
			r_s &= ~sbit(Vf);
			break;

		case 0x86:
			// STX zp
			addr = *pt++;
			goto stx;
		case 0x8e:
			// STX abs
			addr = addr16(*pt++, *pt++);
			goto stx;
		case 0x96:
			// STX zp,y
			addr = ((*pt++) + r_y) & 0xff;
stx:
			vm_ram_write(addr, &r_x, 1);
			break;

		case 0x66:
			// ROR zp
			addr = *pt++;
			goto ror;
		case 0x6a:
			// ROR a
			r_ts = r_a;
			r_a = r_t = ((r_a >> 1) & 0x7f) | (testC(r_s) ? 0x80 : 0);
			goto ror_a;
		case 0x6e:
			// ROR abs
			addr = addr16(*pt++, *pt++);
			goto ror;
		case 0x76:
			// ROR zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto ror;
		case 0x7e:
			// ROR abs,x
			addr = addr16(*pt++, *pt++) + r_x;
ror:
			vm_ram_read(addr, &r_t, 1);
			r_ts = r_t;
			r_t = ((r_t >> 1) & 0x7f) | (testC(r_s) ? 0x80 : 0);
			vm_ram_write(addr, &r_t, 1);
ror_a:
			r_s &= ~(sbit(Nf) | sbit(Zf) | sbit(Cf));
			r_s |= testN(r_t) | testZ(r_t) | testC(r_ts);
			break;

		case 0xd0:
			// BNE
			if (!(r_s & sbit(Zf)))
				r_pc += (int8_t)*pt++;
			else
				r_pc++;
			break;

		case 0x21:
			// AND (zp,x)
			addr = ((*pt++) + r_x) & 0xff;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]);
			goto and;
		case 0x25:
			// AND zp
			addr = *pt++;
			goto and;
		case 0x29:
			// AND #n
			r_a &= *pt++;
			goto and_im;
		case 0x2d:
			// AND abs
			addr = addr16(*pt++, *pt++);
			goto and;
		case 0x31:
			// AND (zp),y
			addr = *pt++;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]) + r_y;
			goto and;
		case 0x35:
			// AND zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto and;
		case 0x39:
			// AND abs,y
			addr = addr16(*pt++, *pt++) + r_y;
			goto and;
		case 0x3d:
			// AND abs,x
			addr = addr16(*pt++, *pt++) + r_x;
and:
			vm_ram_read(addr, &r_t, 1);
			r_a &= r_t;
and_im:
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_s |= testN(r_a) | testZ(r_a);
			break;

		case 0xe8:
			// INX
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_x++;
			r_s |= testN(r_x) | testZ(r_x);
			break;

		case 0xc8:
			// INY
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_y++;
			r_s |= testN(r_y) | testZ(r_y);
			break;

		case 0x28:
			// PLP
			vm_ram_read(addr16(++r_sp, 1), &r_s, 1);
			break;

		case 0x48:
			// PHA
			vm_ram_write(addr16(r_sp--, 1), &r_a, 1);
			break;

		case 0xc1:
			// CMP (zp,x)
			addr = ((*pt++) + r_x) & 0xff;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]);
			goto cmp;
		case 0xc5:
			// CMP zp
			addr = *pt++;
			goto cmp;
		case 0xc9:
			// CMP #n
			r_t = *pt++;
			goto cmp_im;
		case 0xcd:
			// CMP abs
			addr = addr16(*pt++, *pt++);
			goto cmp;
		case 0xd1:
			// CMP (zp),y
			addr = *pt++;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]) + r_y;
			goto cmp;
		case 0xd5:
			// CMP zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto cmp;
		case 0xd9:
			// CMP abs,y
			addr = addr16(*pt++, *pt++) + r_y;
			goto cmp;
		case 0xdd:
			// CMP abs,x
			addr = addr16(*pt++, *pt++) + r_x;
cmp:
			vm_ram_read(addr, &r_t, 1);
cmp_im:
			r_ts = r_a - r_t;
			r_s &= ~(sbit(Nf) | sbit(Zf) | sbit(Cf));
			r_s |= testN(r_ts) | testZ(r_ts) | (r_a >= r_t ? sbit(Cf) : 0);
			break;

		case 0x98:
			// TYA
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_a = r_y;
			r_s |= testN(r_a) | testZ(r_a);
			break;

		case 0x50:
			// BVC
			if (!(r_s & sbit(Vf)))
				r_pc += (int8_t)*pt++;
			else
				r_pc++;
			break;

		case 0xe1:
			// SBC (zp,x)
			addr = ((*pt++) + r_x) & 0xff;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]);
			goto sbc;
		case 0xe5:
			// SBC zp
			addr = *pt++;
			goto sbc;
		case 0xe9:
			// SBC #n
			r_t = *pt++;

			if (testD(r_s))
				goto sbc_im_d;

			goto sbc_im;
		case 0xed:
			// SBC abs
			addr = addr16(*pt++, *pt++);
			goto sbc;
		case 0xf1:
			// SBC (zp),y
			addr = *pt++;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]) + r_y;
			goto sbc;
		case 0xf5:
			// SBC zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto sbc;
		case 0xf9:
			// SBC abs,y
			addr = addr16(*pt++, *pt++) + r_y;
			goto sbc;
		case 0xfd:
			// SBC abs, x
			addr = addr16(*pt++, *pt++) + r_x;
sbc:
			vm_ram_read(addr, &r_t, 1);

sbc_im_d:
			if (testD(r_s))
			{
				uint16_t t;

				r_t16 = (r_a & 0xf) - (r_t & 0xf) - !testC(r_s);
				if (r_t16 & 0x10)
					r_t16 -= 6;
				t = (r_a >> 4) - (r_t >> 4) - ((r_t16 & 0x10) >> 4);
				if (t & 0x10)
					t -= 6;
				r_s &= ~(sbit(Vf) | sbit(Nf) | sbit(Zf) | sbit(Cf));

				r_a = (r_t16 & 0xf) | (t << 4);
				r_s |= (t > 15 ? 0 : sbit(Cf)) | testZ(r_a);
				break;
			}
sbc_im:

			r_t16 = r_a - r_t - !testC(r_s);
			r_s &= ~(sbit(Vf) | sbit(Nf) | sbit(Zf) | sbit(Cf));

			r_s |= (testN((r_a ^ r_t) & (r_a ^ (r_t16 & 0xff))) ? sbit(Vf) : 0)
				| ((r_t16 & 0xff00) ? 0 : sbit(Cf)) | testN(r_t16) | testZ(r_t16 & 0xff);
			r_a = r_t16 & 0xff;
			break;

		case 0x00:
			// BRK
			r_pc += 2;
			pt[0] = r_s | sbit(5) | sbit(Bf);
			pt[1] = (uint8_t)r_pc;
			pt[2] = (uint8_t)(r_pc >> 8);
			vm_ram_write(addr16(r_sp - 2, 1), pt, 3);
			r_sp -= 3;
			vm_ram_read(0xfffe, pt, 2);
			r_pc = addr16(pt[0], pt[1]);
			r_s |= sbit(If);
			return 1;

		case 0x68:
			// PLA
			vm_ram_read(addr16(++r_sp, 1), &r_a, 1);
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_s |= testN(r_a) | testZ(r_a);
			break;

		case 0x08:
			// PHP
			r_t = r_s | sbit(5) | sbit(Bf);
			vm_ram_write(addr16(r_sp--, 1), &r_t, 1);
			break;

		case 0xea:
			// NOP
			break;

		case 0xe6:
			// INC zp
			addr = *pt++;
			goto inc;
		case 0xee:
			// INC abs
			addr = addr16(*pt++, *pt++);
			goto inc;
		case 0xf6:
			// INC zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto inc;
		case 0xfe:
			// INC abs,x
			addr = addr16(*pt++, *pt++) + r_x;
inc:
			vm_ram_read(addr, &r_t, 1);
			r_t++;
			vm_ram_write(addr, &r_t, 1);
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_s |= testN(r_t) | testZ(r_t);
			break;

		case 0x01:
			// ORA (zp,x)
			addr = ((*pt++) + r_x) & 0xff;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]);
			goto ora;
		case 0x05:
			// ORA zp
			addr = *pt++;
			goto ora;
		case 0x09:
			// ORA #n
			r_a |= *pt++;
			goto ora_im;
		case 0x0d:
			// ORA abs
			addr = addr16(*pt++, *pt++);
			goto ora;
		case 0x11:
			// ORA (zp),y
			addr = *pt++;
			vm_ram_read(addr, pt, 2);
			addr = addr16(pt[0], pt[1]) + r_y;
			goto ora;
		case 0x15:
			// ORA zp,x
			addr = ((*pt++) + r_x) & 0xff;
			goto ora;
		case 0x19:
			// ORA abs,y
			addr = addr16(*pt++, *pt++) + r_y;
			goto ora;
		case 0x1d:
			// ORA abs,x
			addr = addr16(*pt++, *pt++) + r_x;
ora:
			vm_ram_read(addr, &r_t, 1);
			r_a |= r_t;
ora_im:
			r_s &= ~(sbit(Nf) | sbit(Zf));
			r_s |= testN(r_a) | testZ(r_a);
			break;

		default:
			// HALT
			return 0;
	}

	r_pc += (pt - _op_cache);

	return 1;
}

