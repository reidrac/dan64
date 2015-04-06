/*
 * dasm.c (standalone version)
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>

#include "dasm.h"
#include "vm.h"

#define VERSION			"1.0"

uint8_t ram[0x10000];

void
dasm_ram_read(uint16_t addr, uint8_t *dst, uint8_t size)
{
	while(size--)
		*dst++ = ram[addr++];
}

void
help(char *argv0)
{
	fprintf(stderr,"DAN64 6502 Assembler & Disassembler\n"
			       "Copyright (C) 2015 Juan J. Martinez <jjm@usebox.net>\n\n"
			       "Usage: %s [-h] [-a address] [-o output] input\n\n"
				   "   input        input filename\n"
				   "   -d           disassemble (default: assemble)\n"
				   "   -s           short format (disassembler only)\n"
				   "   -a address   load address (default: %i)\n"
				   "   -h           this help screen\n"
				   "   -v           print version an exit\n"
				   "   -o output    output filename (default: stdout)\n\n"
				   , argv0, PROG_START);
}

int
main(int argc, char *argv[])
{
	FILE *fdi, *fdo;
	int opt;
	uint16_t addr, len;
	int load_addr = PROG_START;
	char *output = NULL;
	char out[64];
	uint8_t op[3], i;
	int op_len;
	uint8_t das = 0, shortf = 0;

	while ((opt = getopt(argc, argv, "vho:a:ds")) != -1)
	{
		switch(opt)
		{
			case 'd':
				das = 1;
				break;
			case 's':
				shortf = 1;
				break;
			case 'o':
				output = strdup(optarg);
				break;
			case 'a':
				if (sscanf(optarg, "%i", &load_addr) != 1)
				{
					fprintf(stderr, "Failed to process the address\n");
					exit(1);
				}
				break;
			case 'h':
				help(argv[0]);
				exit(0);
			case 'v':
				fprintf(stderr,  VERSION "\n");
				exit(0);
			default:
				fprintf(stderr, "\n");
				help(argv[0]);
				exit(1);
		}
	}

	if (shortf && !das)
	{
		fprintf(stderr, "short format is only available in the disassembler; try -h for help\n");
		exit(1);
	}

	if (optind >= argc)
	{
		fprintf(stderr, "Expected input filename to process; try -h for help\n");
		exit(1);
	}

	fdi = fopen(argv[optind], das ? "rb" : "rt");
	if (!fdi)
	{
		fprintf(stderr, "Failed to open '%s'\n", argv[1]);
		exit(1);
	}

	addr = (uint16_t)load_addr;

	if (!output)
		fdo = stdout;
	else
	{
		fdo = fopen(output, "wt");
		if (!fdo)
		{
			fprintf(stderr, "Failed to open '%s' for output\n", output);
			exit(1);
		}
	}

	if (!das)
	{
		// assembler
		len = 0;
		while(!feof(fdi))
		{
			if(!fgets((char *)ram, 32, fdi))
				break;

			len++;

			op_len = dasm_as(addr, (char *)ram, op);

			if (op_len < 0)
			{
				fprintf(stderr, "%s: line %i, syntax error: %s\n", argv[1], len, ram);
				fprintf(stderr, "%i\n", op_len);
				break;
			}

			fwrite(op, 1, op_len, fdo);
			addr += op_len;
		}
	}
	else
	{
		// always try to load from load addr up the 64KB limit
		len = fread(ram + addr, sizeof(uint8_t), 0x10000 - addr, fdi);
		if (len < 0)
		{
			fprintf(stderr, "Failed to read from '%s'\n", argv[1]);
			fclose(fdi);
			exit(1);
		}

		fclose(fdi);

		// disassembler
		do
		{
			if (!shortf)
				fprintf(fdo, " %04x:  ", addr);
			dasm_ram_read(addr, op, 3);
			op_len = dasm_das(addr, op, out);
			if (!shortf)
			{
				for (i = 0; i < 3; i++)
					if (i < op_len)
						fprintf(fdo, "%02x", op[i]);
					else
						fprintf(fdo, "  ");
			}
			fprintf(fdo, "  %s\n", out);
			addr += op_len;
			len -= op_len;
		} while(addr < 0xffff && len > 0);
	}

	if (output)
		fclose(fdo);

	return 0;
}

