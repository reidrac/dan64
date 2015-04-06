/*
 * dasm.c
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
#include "dasm.h"

#define OPCODES		153

// disassembler table follows
//     opcode: 1 byte
//   mnemonic: 3 bytes
// addressing: 1 byte
const uint8_t vm_op_tbl[] PROGMEM = {
	0x00, 'B', 'R', 'K', AT_IMPLIED, // BRK
	0x01, 'O', 'R', 'A', AT_ZP_INDEX_X, // ORA
	0x02, 'S', 'Y', 'S', AT_IMPLIED, // SYS
	0x05, 'O', 'R', 'A', AT_ZEROP, // ORA
	0x06, 'A', 'S', 'L', AT_ZEROP, // ASL
	0x08, 'P', 'H', 'P', AT_IMPLIED, // PHP
	0x09, 'O', 'R', 'A', AT_IMMEDIATE, // ORA
	0x0a, 'A', 'S', 'L', AT_ACCUMULATOR, // ASL
	0x0d, 'O', 'R', 'A', AT_ABSOLUTE, // ORA
	0x0e, 'A', 'S', 'L', AT_ABSOLUTE, // ASL
	0x10, 'B', 'P', 'L', AT_RELATIVE, // BPL
	0x11, 'O', 'R', 'A', AT_IND_INDEX, // ORA
	0x15, 'O', 'R', 'A', AT_ZP_INDEX_X, // ORA
	0x16, 'A', 'S', 'L', AT_ZP_INDEX_X, // ASL
	0x18, 'C', 'L', 'C', AT_IMPLIED, // CLC
	0x19, 'O', 'R', 'A', AT_ABS_INDEX_Y, // ORA
	0x1d, 'O', 'R', 'A', AT_ABS_INDEX_X, // ORA
	0x1e, 'A', 'S', 'L', AT_ABS_INDEX_X, // ASL
	0x20, 'J', 'S', 'R', AT_ABSOLUTE, // JSR
	0x21, 'A', 'N', 'D', AT_ZP_INDEX_X, // AND
	0x24, 'B', 'I', 'T', AT_ZEROP, // BIT
	0x25, 'A', 'N', 'D', AT_ZEROP, // AND
	0x26, 'R', 'O', 'L', AT_ZEROP, // ROL
	0x28, 'P', 'L', 'P', AT_IMPLIED, // PLP
	0x29, 'A', 'N', 'D', AT_IMMEDIATE, // AND
	0x2a, 'R', 'O', 'L', AT_ACCUMULATOR, // ROL
	0x2c, 'B', 'I', 'T', AT_ABSOLUTE, // BIT
	0x2d, 'A', 'N', 'D', AT_ABSOLUTE, // AND
	0x2e, 'R', 'O', 'L', AT_ABSOLUTE, // ROL
	0x30, 'B', 'M', 'I', AT_RELATIVE, // BMI
	0x31, 'A', 'N', 'D', AT_IND_INDEX, // AND
	0x35, 'A', 'N', 'D', AT_ZP_INDEX_X, // AND
	0x36, 'R', 'O', 'L', AT_ZP_INDEX_X, // ROL
	0x38, 'S', 'E', 'C', AT_IMPLIED, // SEC
	0x39, 'A', 'N', 'D', AT_ABS_INDEX_Y, // AND
	0x3d, 'O', 'R', 'A', AT_ABS_INDEX_X, // ORA
	0x3e, 'R', 'O', 'L', AT_ABS_INDEX_X, // ROL
	0x40, 'R', 'T', 'I', AT_IMPLIED, // RTI
	0x41, 'E', 'O', 'R', AT_INDEX_IND, // EOR
	0x45, 'E', 'O', 'R', AT_ZEROP, // EOR
	0x46, 'L', 'S', 'R', AT_ZEROP, // LSR
	0x48, 'P', 'H', 'A', AT_IMPLIED, // PHA
	0x49, 'E', 'O', 'R', AT_IMMEDIATE, // EOR
	0x4a, 'L', 'S', 'R', AT_ACCUMULATOR, // LSR
	0x4c, 'J', 'M', 'P', AT_ABSOLUTE, // JMP
	0x4d, 'E', 'O', 'R', AT_ABSOLUTE, // EOR
	0x4e, 'L', 'S', 'R', AT_ABSOLUTE, // LSR
	0x50, 'B', 'V', 'C', AT_RELATIVE, // BVC
	0x51, 'E', 'O', 'R', AT_IND_INDEX, // EOR
	0x55, 'E', 'O', 'R', AT_ZP_INDEX_X, // EOR
	0x56, 'L', 'S', 'R', AT_ZP_INDEX_X, // LSR
	0x58, 'C', 'L', 'I', AT_IMPLIED, // CLI
	0x59, 'E', 'O', 'R', AT_ABS_INDEX_Y, // EOR
	0x5d, 'E', 'O', 'R', AT_ABS_INDEX_X, // EOR
	0x5e, 'L', 'S', 'R', AT_ABS_INDEX_X, // LSR
	0x60, 'R', 'T', 'S', AT_IMPLIED, // RTS
	0x61, 'A', 'D', 'C', AT_ZP_INDEX_X, // ADC
	0x65, 'A', 'D', 'C', AT_ZEROP, // ADC
	0x66, 'R', 'O', 'R', AT_ZEROP, // ROR
	0x68, 'P', 'L', 'A', AT_IMPLIED, // PLA
	0x69, 'A', 'D', 'C', AT_IMMEDIATE, // ADC
	0x6a, 'R', 'O', 'R', AT_ACCUMULATOR, // ROR
	0x6c, 'J', 'M', 'P', AT_IND_ABS, // JMP
	0x6d, 'A', 'D', 'C', AT_ABSOLUTE, // ADC
	0x6e, 'R', 'O', 'R', AT_ABSOLUTE, // ROR
	0x70, 'B', 'V', 'S', AT_RELATIVE, // BVS
	0x71, 'A', 'D', 'C', AT_IND_INDEX, // ADC
	0x75, 'A', 'D', 'C', AT_ZP_INDEX_X, // ADC
	0x76, 'R', 'O', 'R', AT_ZP_INDEX_X, // ROR
	0x78, 'S', 'E', 'I', AT_IMPLIED, // SEI
	0x79, 'A', 'D', 'C', AT_ABS_INDEX_Y, // ADC
	0x7d, 'A', 'D', 'C', AT_ABS_INDEX_X, // ADC
	0x7e, 'R', 'O', 'R', AT_ABS_INDEX_X, // ROR
	0x81, 'S', 'T', 'A', AT_INDEX_IND, // STA
	0x84, 'S', 'T', 'Y', AT_ZEROP, // STY
	0x85, 'S', 'T', 'A', AT_ZEROP, // STA
	0x86, 'S', 'T', 'X', AT_ZEROP, // STX
	0x88, 'D', 'E', 'Y', AT_IMPLIED, // DEY
	0x8a, 'T', 'X', 'A', AT_IMPLIED, // TXA
	0x8c, 'S', 'T', 'Y', AT_ABSOLUTE, // STY
	0x8d, 'S', 'T', 'A', AT_ABSOLUTE, // STA
	0x8e, 'S', 'T', 'X', AT_ABSOLUTE, // STX
	0x90, 'B', 'C', 'C', AT_RELATIVE, // BCC
	0x91, 'S', 'T', 'A', AT_IND_INDEX, // STA
	0x94, 'S', 'T', 'Y', AT_ZP_INDEX_X, // STY
	0x95, 'S', 'T', 'A', AT_ZP_INDEX_X, // STA
	0x96, 'S', 'T', 'X', AT_ZP_INDEX_Y, // STX
	0x98, 'T', 'Y', 'A', AT_IMPLIED, // TYA
	0x99, 'S', 'T', 'A', AT_ABS_INDEX_Y, // STA
	0x9a, 'T', 'X', 'S', AT_IMPLIED, // TXS
	0x9d, 'S', 'T', 'A', AT_ABS_INDEX_X, // STA
	0xa0, 'L', 'D', 'Y', AT_IMMEDIATE, // LDY
	0xa1, 'L', 'D', 'A', AT_ZP_INDEX_X, // LDA
	0xa2, 'L', 'D', 'X', AT_IMMEDIATE, // LDX
	0xa4, 'L', 'D', 'Y', AT_ZEROP, // LDY
	0xa5, 'L', 'D', 'A', AT_ZEROP, // LDA
	0xa6, 'L', 'D', 'X', AT_ZEROP, // LDX
	0xa8, 'T', 'A', 'Y', AT_IMPLIED, // TAY
	0xa9, 'L', 'D', 'A', AT_IMMEDIATE, // LDA
	0xaa, 'T', 'A', 'X', AT_IMPLIED, // TAX
	0xac, 'L', 'D', 'Y', AT_ABSOLUTE, // LDY
	0xad, 'L', 'D', 'A', AT_ABSOLUTE, // LDA
	0xae, 'L', 'D', 'X', AT_ABSOLUTE, // LDX
	0xb0, 'B', 'C', 'S', AT_RELATIVE, // BCS
	0xb1, 'L', 'D', 'A', AT_IND_INDEX, // LDA
	0xb4, 'L', 'D', 'Y', AT_ZP_INDEX_X, // LDY
	0xb5, 'L', 'D', 'A', AT_ZP_INDEX_X, // LDA
	0xb6, 'L', 'D', 'X', AT_ZP_INDEX_Y, // LDX
	0xb8, 'C', 'L', 'V', AT_IMPLIED, // CLV
	0xb9, 'L', 'D', 'A', AT_ABS_INDEX_Y, // LDA
	0xba, 'T', 'S', 'X', AT_IMPLIED, // TSX
	0xbc, 'L', 'D', 'Y', AT_ABS_INDEX_X, // LDY
	0xbd, 'L', 'D', 'A', AT_ABS_INDEX_X, // LDA
	0xbe, 'L', 'D', 'X', AT_ABS_INDEX_Y, // LDX
	0xc0, 'C', 'P', 'Y', AT_IMMEDIATE, // CPY
	0xc1, 'C', 'M', 'P', AT_ZP_INDEX_X, // CMP
	0xc4, 'C', 'P', 'Y', AT_ZEROP, // CPY
	0xc5, 'C', 'M', 'P', AT_ZEROP, // CMP
	0xc6, 'D', 'E', 'C', AT_ZEROP, // DEC
	0xc8, 'I', 'N', 'Y', AT_IMPLIED, // INY
	0xc9, 'C', 'M', 'P', AT_IMMEDIATE, // CMP
	0xca, 'D', 'E', 'X', AT_IMPLIED, // DEX
	0xcc, 'C', 'P', 'Y', AT_ABSOLUTE, // CPY
	0xcd, 'C', 'M', 'P', AT_ABSOLUTE, // CMP
	0xce, 'D', 'E', 'C', AT_ABSOLUTE, // DEC
	0xd0, 'B', 'N', 'E', AT_RELATIVE, // BNE
	0xd1, 'C', 'M', 'P', AT_IND_INDEX, // CMP
	0xd5, 'C', 'M', 'P', AT_ZP_INDEX_X, // CMP
	0xd6, 'D', 'E', 'C', AT_ZP_INDEX_X, // DEC
	0xd8, 'C', 'L', 'D', AT_IMPLIED, // CLD
	0xd9, 'C', 'M', 'P', AT_ABS_INDEX_Y, // CMP
	0xdd, 'C', 'M', 'P', AT_ABS_INDEX_X, // CMP
	0xde, 'D', 'E', 'C', AT_ABS_INDEX_X, // DEC
	0xe0, 'C', 'P', 'X', AT_IMMEDIATE, // CPX
	0xe1, 'S', 'B', 'C', AT_ZP_INDEX_X, // SBC
	0xe4, 'C', 'P', 'X', AT_ZEROP, // CPX
	0xe5, 'S', 'B', 'C', AT_ZEROP, // SBC
	0xe6, 'I', 'N', 'C', AT_ZEROP, // INC
	0xe8, 'I', 'N', 'X', AT_IMPLIED, // INX
	0xe9, 'S', 'B', 'C', AT_IMMEDIATE, // SBC
	0xea, 'N', 'O', 'P', AT_IMPLIED, // NOP
	0xec, 'C', 'P', 'X', AT_ABSOLUTE, // CPX
	0xed, 'S', 'B', 'C', AT_ABSOLUTE, // SBC
	0xee, 'I', 'N', 'C', AT_ABSOLUTE, // INC
	0xf0, 'B', 'E', 'Q', AT_RELATIVE, // BEQ
	0xf1, 'S', 'B', 'C', AT_IND_INDEX, // SBC
	0xf5, 'S', 'B', 'C', AT_ZP_INDEX_X, // SBC
	0xf6, 'I', 'N', 'C', AT_ZP_INDEX_X, // INC
	0xf8, 'S', 'E', 'D', AT_IMPLIED, // SED
	0xf9, 'S', 'B', 'C', AT_ABS_INDEX_Y, // SBC
	0xfd, 'S', 'B', 'C', AT_ABS_INDEX_X, // SBC
	0xfe, 'I', 'N', 'C', AT_ABS_INDEX_X // INC
};

static char *
skip_whitespace(char *p)
{
	while (*p && isspace(*p))
		p++;

	return p;
}

static uint8_t
find_mne(uint8_t *mne, int addr_mode)
{
	uint8_t i;

	for (i = 0; i < OPCODES; i++)
		if (!memcmp_P(mne, &vm_op_tbl[i * 5 + 1], 3))
			if (addr_mode < 0 || pgm_read_byte(&vm_op_tbl[i * 5 + 4]) == addr_mode)
				break;

	return i;
}

static void inline
emit_op(uint8_t *output, uint8_t index, uint16_t val, uint8_t len)
{
	*output++ = pgm_read_byte(&vm_op_tbl[index * 5]);
	if (len > 1)
		*output++ = (uint8_t)val;
	if (len > 2)
		*output++ = (uint8_t)(val>> 8);
}

static int
get_value(char *input, uint16_t *val)
{
	uint8_t len = 0;
	int res, v;

	if (!*input)
		return -1;

	if (*input == '$')
	{
		if (!input[1])
			return -1;
		len++;
		res = sscanf(input + 1, "%x", val);
		for (; input[len] && isxdigit(input[len]); len++);
	}
	else
	{
		res = sscanf(input, "%d", &v);
		*val = (uint16_t)v;
		for (; input[len] && isdigit(input[len]); len++);
	}

	if (res != 1)
		return -1;

	return len;
}

int
dasm_as(uint16_t addr, char *input, uint8_t *output)
{
	char *p = input, *mne;
	int val_len;
	uint8_t i, index, len = 0, mode = 0;
	uint16_t val;

	p = skip_whitespace(p);
	if (!*p)
		return -1;

	if (*p == '.')
	{
		// special cases
		p++;
		if (!*p)
			return -1;

		while (*p)
		{
			switch(*p)
			{
				case 0:
					// done
					break;

				case '"':
					// string
					p++;
					while (*p && *p != '"')
					{
						if (p[0] == '\\' && p[1] == '"')
							p++;

						*output++ = *p++;
						len++;
					}
					if (*p != '"')
						return -1;
					p++;
					break;

				case '0' ... '9':
				case '$':
					// byte
					if ((val_len = get_value(p, &val)) < 0)
						return -1;
					if (val > 0xff)
						return -1;

					*output++ = val;
					len++;
					p += val_len;
					break;

				case ',':
					// separator
					p++;
					break;

				default:
					return -1;
			}

			p = skip_whitespace(p);
		}

		return len;
	}

	if (strlen(p) < 3)
		return -1;

	// convert to upper case
	for (i = 0; p[i]; i++)
		if (p[i] >= 'a' && p[i] <= 'z')
			p[i] = toupper(p[i]);

	// find a matching mnemonic using any addressing mode
	index = find_mne((uint8_t *)p, -1);
	if (index == OPCODES)
		return -1;

	mne = p;

	p = skip_whitespace(p + 3);
	switch (*p)
	{
		case 0:
			// implied
			mode = AT_IMPLIED;
			len = 1;
			break;

		case 'A':
			// accumulator
			p = skip_whitespace(p + 1);
			if (*p)
				return -1;

			mode = AT_ACCUMULATOR;
			len = 1;
			break;

		case '#':
			// immediate
			mode = AT_IMMEDIATE;
			len = 2;
			if ((val_len = get_value(++p, &val)) < 0)
				return -1;
			if (val > 0xff)
				return -1;

			p = skip_whitespace(p + val_len);
			if (*p)
				return -1;
			break;

		case '(':
			// indirect something
			if ((val_len = get_value(++p, &val)) < 0)
				return -1;

			p = skip_whitespace(p + val_len);
			switch (*p)
			{
				case ')':
					p = skip_whitespace(p + 1);
					if (!*p)
					{
						mode = AT_IND_ABS;
						len = 3;
					}
					else
					{
						if (*p++ != ',')
							return -1;

						if (*p++ != 'Y')
							return -1;

						p = skip_whitespace(p);
						if (*p)
							return -1;

						mode = AT_IND_INDEX;
						len = 2;

						if (val > 0xff)
							return -1;
					}
					break;

				case ',':
					p = skip_whitespace(p + 1);
					if (!*p)
						return -1;

					if (*p++ != 'X')
						return -1;

					p = skip_whitespace(p);
					if (!*p)
						return -1;

					if (*p++ != ')')
						return -1;

					p = skip_whitespace(p);
					if (*p)
						return -1;

					mode = AT_INDEX_IND;
					len = 2;

					if (val > 0xff)
						return -1;
					break;
			}
			break;

		case '0' ... '9':
		case '$':
			// absolute, relative, zp or indexed
			if ((val_len = get_value(p, &val)) < 0)
				return -1;

			p = skip_whitespace(p + val_len);
			switch(*p)
			{
				case 0:
					// absolute, relative or zp
					mode = AT_ABSOLUTE;
					len = 3;

					if (val <= 0xff && find_mne((uint8_t *)mne, AT_ZEROP) != OPCODES)
					{
						mode = AT_ZEROP;
						len = 2;
					}
					else
					{
						if (find_mne((uint8_t *)mne, AT_RELATIVE) != OPCODES)
						{
							mode = AT_RELATIVE;
							len = 2;
							val = (uint8_t)(val - addr - 1);

							if (val > 0xff)
								return -1;
						}
					}
					break;

				case ',':
					// absolute or zp indexed
					p = skip_whitespace(p + 1);
					switch (*p++)
					{
						case 'X':
							if (val <= 0xff && find_mne((uint8_t *)mne, AT_ZP_INDEX_X) != OPCODES)
							{
								mode = AT_ZP_INDEX_X;
								len = 2;
							}
							else
							{
								mode = AT_ABS_INDEX_X;
								len = 3;
							}
							break;

						case 'Y':
							if (val <= 0xff && find_mne((uint8_t *)mne, AT_ZP_INDEX_Y) != OPCODES)
							{
								mode = AT_ZP_INDEX_Y;
								len = 2;
							}
							else
							{
								mode = AT_ABS_INDEX_Y;
								len = 3;
							}
							break;

						default:
							return -1;
					}

					p = skip_whitespace(p);
					if (*p)
						return -1;
					break;

				default:
					return -1;
			}
			break;

		default:
			return -1;
	}

	index = find_mne((uint8_t *)mne, mode);
	if (index == OPCODES)
		return -1;

	emit_op(output, index, val, len);
	return len;
}

uint8_t
dasm_das(uint16_t addr, uint8_t *op, char *output)
{
	uint8_t ent[5], i;

	for (i = 0; i < OPCODES; i++)
	{
		memcpy_P(ent, &vm_op_tbl[i * 5], 5);

		if (ent[0] == op[0])
			break;
	}

	if (i == OPCODES)
	{
		strcpy(output, "???");
		return 1;
	}

	// opcode string
	for (i = 0; i < 3; i++)
		output[i] = ent[i + 1];
	output[i] = 0;

	if (ent[4] == AT_IMPLIED)
		return 1;
	else
		strcat(output, " ");

	// addressing mode
	switch (ent[4])
	{
		case AT_ABSOLUTE:
			sprintf(output + 4, "$%04x", addr16(op[1], op[2]));
			return 3;

		case AT_IMMEDIATE:
			sprintf(output + 4, "#$%02x", op[1]);
			return 2;

		case AT_ZEROP:
			sprintf(output + 4, "$%02x", op[1]);
			return 2;

		case AT_IND_ABS:
			sprintf(output + 4, "($%04x)", addr16(op[1], op[2]));
			return 3;

		case AT_ABS_INDEX_X:
			sprintf(output + 4, "$%04x,X", addr16(op[1], op[2]));
			return 3;

		case AT_ABS_INDEX_Y:
			sprintf(output + 4, "$%04x,Y", addr16(op[1], op[2]));
			return 3;

		case AT_ZP_INDEX_X:
			sprintf(output + 4, "$%02x,X", op[1]);
			return 2;

		case AT_ZP_INDEX_Y:
			sprintf(output + 4, "$%02x,Y", op[1]);
			return 2;

		case AT_INDEX_IND:
			sprintf(output + 4, "($%02x,X)", op[1]);
			return 2;

		case AT_IND_INDEX:
			sprintf(output + 4, "($%02x),Y", op[1]);
			return 2;

		case AT_RELATIVE:
			sprintf(output + 4, "$%04x", addr + 1 + (int8_t)op[1]);
			return 2;

		case AT_ACCUMULATOR:
			strcat(output, "A");
			return 1;
	}

	// shouldn't happen
	return 1;
}

