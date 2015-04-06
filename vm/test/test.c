#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include "op_nm.h"

uint8_t ram[65536];

extern uint8_t r_a, r_x, r_y, r_sp, r_s;
extern uint16_t r_pc;

extern uint8_t op, r_t, r_ts;

inline void
vm_ram_read(uint16_t addr, uint8_t *dst, uint8_t size)
{
	while(size--)
		*dst++ = ram[addr++];
}

inline void
vm_ram_write(uint16_t addr, uint8_t *src, uint8_t size)
{
	while(size--)
		ram[addr++] = *src++;
}

inline void
vm_syscall(uint8_t func)
{
	// do nothing
}

void
dump_regs()
{
	uint8_t i;

	fprintf(stderr, "  PC: %04x SP: %02x S: %02x (N%iV%i-B%iD%iI%iZ%iC%i)\n"
			        "  A: %02x X: %02x Y: %02x\n\n",
					r_pc, r_sp, r_s,
					(sbit(Nf) & r_s) != 0,
					(sbit(Vf) & r_s) != 0,
					(sbit(Bf) & r_s) != 0,
					(sbit(Df) & r_s) != 0,
					(sbit(If) & r_s) != 0,
					(sbit(Zf) & r_s) != 0,
					(sbit(Cf) & r_s) != 0,
					r_a, r_x, r_y);

	fprintf(stderr, "  %04x: ", r_pc);
	for(i = 0; i < 16 && i + r_pc <= 0xffff; i++)
		fprintf(stderr, "%02x ", ram[r_pc + i]);
	fprintf(stderr, "\n");
	fprintf(stderr, "****\n");
}

int
main()
{
	const char *tests[1] = { "images/6502_functional_test.bin" /* add more? */ };
	uint16_t results[1] = { 0x3399 };
	FILE *fd;
	uint8_t i;
	uint32_t old = 0, new = 1, ops = 0, ok;

	for (i = 0; i < 1; i++)
	{
		printf("** Test: %s\n", tests[i]);

		fd = fopen(tests[i], "rb");
		if (!fd)
		{
			fprintf(stderr, "failed to open %s\n", tests[i]);
			return 1;
		}

		if (fread(ram, 1, 0x10000, fd) <= 0)
		{
			fprintf(stderr, "failed to read from %s\n", tests[i]);
			fclose(fd);
			return 1;
		}

		fclose(fd);

		vm_init();
		r_pc = 0x400;

		ok = 0;
		while(vm_exec())
		{
			ops++;
			new = r_pc | (r_sp << 16) | (r_s << 24);
			if (old == new)
			{
				fprintf(stderr, "TRAP detected!\n\n");
				fprintf(stderr, "  %04x: %02x    %c%c%c\n\n", r_pc, op,
						op_nm[op][0], op_nm[op][1], op_nm[op][2]);
				dump_regs();
				fprintf(stderr, "OPS run: %ul\n", ops);
				ok = 1;
				break;
			}
			old = new;
		}

		if (!ok)
		{
			fprintf(stderr, "Invalid op!\n\n");
			fprintf(stderr, "  %04x: %02x    %c%c%c\n\n", r_pc, op,
					op_nm[op][0], op_nm[op][1], op_nm[op][2]);
			dump_regs();
			fprintf(stderr, "OPS run: %ul\n", ops);
			printf("** Error\n");
			return 1;
		}
		else
		{
			if (r_pc == results[i])
				printf("** Ok\n");
			else
			{
				printf("** Failed\n");
				return 1;
			}
		}
	}

	return 0;
}

