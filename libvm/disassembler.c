#include "disassembler.h"
#include "mnemonics.h"
#include <string.h>
#include <stdlib.h>

struct disassembler_options disasmOptions =
{
	false,
	true
};

static const char *commandStrings[] =
{
	// VM_CMD_COPY     0
		"copy",
		// VM_CMD_STORE    1
			"store",
	// VM_CMD_LOAD     2  
		"load",
	// VM_CMD_GET      3
		"get",
	// VM_CMD_SET      4
		"set",
	// VM_CMD_BPGET    5
		"bpget",
	// VM_CMD_BPSET    6
		"bpset",
	// VM_CMD_CPGET    7
		"cpget",
	// VM_CMD_MATH     8
		"math",
	// VM_CMD_SPGET    9
		"spget",
	// VM_CMD_SPSET    10
		"spset",
	// VM_CMD_SYSCALL  11
		"syscall",
	// VM_CMD_HWIO     12
		"hwio",
};

void disassemble(instruction_t *list, uint32_t count, uint32_t base, FILE *f)
{
	bool v = disasmOptions.verbose;
	for (int i = 0; i < count; i++)
	{
		instruction_t instr = list[i];
		if(disasmOptions.outputAddresses)
			fprintf(f, "%8X:  ", base + i);

		const mnemonic_t *knownInstruction = NULL;

		for (int j = 0; mnemonics[j].name != NULL; j++)
		{
			if (memcmp(&instr, &mnemonics[j].instr, sizeof(instruction_t) - sizeof(uint32_t)) == 0) {
				knownInstruction = &mnemonics[j];
				break;
			}
		}

		if (knownInstruction != NULL)
		{
			fprintf(f, "%s", knownInstruction->name);
			if (instr.argument != 0 || instr.input0 == VM_INPUT_ARG)
			{
				if(instr.output == VM_OUTPUT_JUMP || instr.output == VM_OUTPUT_JUMPR)
					fprintf(f, " 0x%X", instr.argument);
				else
					fprintf(f, " %d", instr.argument);
			}
			fprintf(f, "\n");
			continue;
		}

		switch (instr.execN)
		{
		case VM_EXEC_0: fprintf(f, "[ex(n)=0] "); break;
		case VM_EXEC_1: fprintf(f, "[ex(n)=1] "); break;
		case VM_EXEC_X: if (v) fprintf(f, "[ex(n)=x] "); break;
		default: fprintf(stderr, "Invalid code @%d\n", base + i); abort();
		}

		switch (instr.execZ)
		{
		case VM_EXEC_0: fprintf(f, "[ex(z)=0] "); break;
		case VM_EXEC_1: fprintf(f, "[ex(z)=1] "); break;
		case VM_EXEC_X: if (v) fprintf(f, "[ex(z)=x] "); break;
		default: fprintf(stderr, "Invalid code @%d\n", base + i); abort();
		}

		switch (instr.input0)
		{
		case VM_INPUT_ZERO: if (v) fprintf(f, "[i0:zero] "); break;
		case VM_INPUT_POP: fprintf(f, "[i0:pop] "); break;
		case VM_INPUT_PEEK: fprintf(f, "[i0:peek] "); break;
		case VM_INPUT_ARG: fprintf(f, "[i0:arg] "); break;
		default: fprintf(stderr, "Invalid code @%d\n", base + i); abort();
		}

		switch (instr.input1)
		{
		case VM_INPUT_ZERO: if (v) fprintf(f, "[i1:zero] "); break;
		case VM_INPUT_POP: fprintf(f, "[i1:pop] "); break;
		// case VM_INPUT_PEEK: fprintf(f, "[i1:peek] "); break;
		// case VM_INPUT_ARG: fprintf(f, "[i1:arg] "); break;
		default: fprintf(stderr, "Invalid code @%d\n", base + i); abort();
		}

		if (instr.command <= 12)
			fprintf(f, "%s", commandStrings[instr.command]);
		else
			fprintf(f, "undefined [cmd:%d]", instr.command);

		if (instr.cmdinfo != 0)
		{
			fprintf(f, " [ci:%d]", instr.cmdinfo);
		}

		if (instr.argument != 0 || instr.input0 == VM_INPUT_ARG)
		{
			if (instr.output == VM_OUTPUT_JUMP || instr.output == VM_OUTPUT_JUMPR)
				fprintf(f, " 0x%X", instr.argument);
			else
				fprintf(f, " %d", instr.argument);
		}

		switch (instr.flags)
		{
		case VM_FLAG_NO: if (v) fprintf(f, " [f:no]"); break;
		case VM_FLAG_YES: fprintf(f, " [f:yes]"); break;
		default: fprintf(stderr, "Invalid code @%d\n", base + i); abort();
		}

		switch (instr.output)
		{
		case VM_OUTPUT_DISCARD: fprintf(f, " [r:discard]"); break;
		case VM_OUTPUT_JUMP: fprintf(f, " [r:jump]"); break;
		case VM_OUTPUT_JUMPR: fprintf(f, " [r:jumpr]"); break;
		case VM_OUTPUT_PUSH: fprintf(f, " [r:push]"); break;
		default: fprintf(stderr, "Invalid code @%d\n", base + i); abort();
		}

		fprintf(f, "\n");
	}
}