#if !defined(__gcc)
#define  _CRT_SECURE_NO_WARNINGS
#endif

#include <stdint.h>
#include <stddef.h>

#include "exp.h"
#include "vm.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "getopt.h"
#else
#include <getopt.h>
#endif

#include <string.h>
#include <ctype.h>

#define DEBUG_VAL(x) fprintf(stderr, #x " = %d\n", x)

struct PredefinedCmd
{
	const char *name;
	Instruction instr;
};

struct PredefinedCmd namedCommands[] =
{
	{ "nop",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_ZERO, VM_INPUT_ZERO, VM_CMD_COPY, 0, VM_FLAG_NO, VM_OUTPUT_DISCARD } },
	{ "push",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_ARG, VM_INPUT_ZERO, VM_CMD_COPY, 0, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "drop",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_ZERO, VM_CMD_COPY, 0, VM_FLAG_NO, VM_OUTPUT_DISCARD } },
	{ "dup",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_PEEK, VM_INPUT_ZERO, VM_CMD_COPY, 0, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "jmp",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_ARG, VM_INPUT_ZERO, VM_CMD_COPY, 0, VM_FLAG_NO, VM_OUTPUT_JUMP } },
	{ "jmpi",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_ZERO, VM_CMD_COPY, 0, VM_FLAG_NO, VM_OUTPUT_JUMP } },
	{ "ret",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_ZERO, VM_CMD_COPY, 0, VM_FLAG_NO, VM_OUTPUT_JUMP } },
	{ "load",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_ARG, VM_INPUT_ZERO, VM_CMD_LOAD, 0, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "loadi",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_ZERO, VM_CMD_LOAD, 0, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "store",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_ARG, VM_INPUT_POP, VM_CMD_STORE, 0, VM_FLAG_NO, VM_OUTPUT_DISCARD } },
	{ "storei",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_STORE, 0, VM_FLAG_NO, VM_OUTPUT_DISCARD } },
	{ "get",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_ARG, VM_INPUT_ZERO, VM_CMD_GET, 0, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "geti",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_ZERO, VM_CMD_GET, 0, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "set",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_ARG, VM_INPUT_POP, VM_CMD_SET, 0, VM_FLAG_NO, VM_OUTPUT_DISCARD } },
	{ "seti",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_SET, 0, VM_FLAG_NO, VM_OUTPUT_DISCARD } },
	{ "bpget",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_ZERO, VM_INPUT_ZERO, VM_CMD_BPGET, 0, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "bpset",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_ZERO, VM_CMD_BPSET, 0, VM_FLAG_NO, VM_OUTPUT_DISCARD } },
	{ "spget",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_ZERO, VM_INPUT_ZERO, VM_CMD_SPGET, 0, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "spset",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_ZERO, VM_CMD_SPSET, 0, VM_FLAG_NO, VM_OUTPUT_DISCARD } },
	{ "cpget",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_ZERO, VM_INPUT_ZERO, VM_CMD_CPGET, 1, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "add",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 0, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "sub",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 1, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "cmp",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 1, VM_FLAG_YES, VM_OUTPUT_DISCARD } },
	{ "mul",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 2, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "div",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 3, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "mod",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 4, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "and",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 5, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "or",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 6, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "xor",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 7, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "not",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_ZERO, VM_CMD_MATH, 8, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "rol",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 9, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "ror",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 10, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "asl",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 11, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "asr",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 12, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "shl",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 13, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "shr",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_POP, VM_INPUT_POP, VM_CMD_MATH, 14, VM_FLAG_NO, VM_OUTPUT_PUSH } },
	{ "syscall",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_ZERO, VM_INPUT_ZERO, VM_CMD_SYSCALL, 0, VM_FLAG_NO, VM_OUTPUT_DISCARD } },
	{ "hwio",{ VM_EXEC_X, VM_EXEC_X, VM_INPUT_ZERO, VM_INPUT_ZERO, VM_CMD_HWIO, 0, VM_FLAG_NO, VM_OUTPUT_DISCARD } },

};

const char *commandStrings[] =
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

int disassembleVerbose = 0;

void disassemble(Instruction *list, uint32_t count, uint32_t base, FILE *f)
{
	int v = disassembleVerbose;
	for (int i = 0; i < count; i++)
	{
		Instruction instr = list[i];

		fprintf(f, "%8X:  ", base + i);

		struct PredefinedCmd *knownInstruction = NULL;

		for (int j = 0; j < sizeof(namedCommands) / sizeof(struct PredefinedCmd); j++)
		{
			if (memcmp(&instr, &namedCommands[j].instr, sizeof(Instruction) - sizeof(uint32_t)) == 0) {
				knownInstruction = &namedCommands[j];
				break;
			}
		}

		if (knownInstruction != NULL)
		{
			fprintf(f, "%s", knownInstruction->name);
			if (instr.input0 == VM_INPUT_ARG)
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
		}

		switch (instr.execZ)
		{
		case VM_EXEC_0: fprintf(f, "[ex(z)=0] "); break;
		case VM_EXEC_1: fprintf(f, "[ex(z)=1] "); break;
		case VM_EXEC_X: if (v) fprintf(f, "[ex(z)=x] "); break;
		}

		switch (instr.input0)
		{
		case VM_INPUT_ZERO: if (v) fprintf(f, "[i0:zero] "); break;
		case VM_INPUT_POP: fprintf(f, "[i0:pop] "); break;
		case VM_INPUT_PEEK: fprintf(f, "[i0:peek] "); break;
		case VM_INPUT_ARG: fprintf(f, "[i0:arg] "); break;
		}

		switch (instr.input1)
		{
		case VM_INPUT_ZERO: if (v) fprintf(f, "[i1:zero] "); break;
		case VM_INPUT_POP: fprintf(f, "[i1:pop] "); break;
		case VM_INPUT_PEEK: fprintf(f, "[i1:peek] "); break;
		case VM_INPUT_ARG: fprintf(f, "[i1:arg] "); break;
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
		}

		switch (instr.output)
		{
		case VM_OUTPUT_DISCARD: if (v) fprintf(f, " [r:discard]"); break;
		case VM_OUTPUT_JUMP: fprintf(f, " [r:jump]"); break;
		case VM_OUTPUT_JUMPR: fprintf(f, " [r:jumpr]"); break;
		case VM_OUTPUT_PUSH: fprintf(f, " [r:push]"); break;
		}

		fprintf(f, "\n");
	}
}


int main(int argc, char **argv)
{
	opterr = 0;

	int dumpSections = 0;
	int dumpMetas = 0;

	int disassembleSections = 0;

	int c;
	while ((c = getopt(argc, argv, "smdD")) != -1)
	{
		switch (c)
		{
		case 's': dumpSections = 1; break;
		case 'm': dumpMetas = 1; break;
		case 'D': disassembleVerbose = 1;
		case 'd': disassembleSections = 1; break;
		case '?':
			if (optopt == 'o' || optopt == 'c' || optopt == 'd')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			return 1;
		default:
			abort();
		}
	}
	for (int index = optind; index < argc; index++)
	{
		const char *fileName = argv[index];
		FILE *f = fopen(fileName, "rb");
		if (f == NULL)
		{
			fprintf(stderr, "Could not open file %s\n", fileName);
			continue;
		}

		///////////////////////////////////////////////////////////////////////////////

		expfile_t fileHeader;

		if (fread(&fileHeader, 1, sizeof(expfile_t), f) != sizeof(expfile_t))
		{
			fprintf(stderr, "File %s does not contain a valid header.\n", fileName);
			continue;
		}
		if (fileHeader.magicNumber != EXP_MAGIC)
		{
			fprintf(
				stderr, "Invalid magic in %s\n",
				fileHeader.majorVersion, fileHeader.minorVersion);
			continue;
		}
		if (fileHeader.majorVersion != 1 && fileHeader.minorVersion == 0)
		{
			fprintf(
				stderr, "Invalid version %s: %d.%d\n",
				fileHeader.majorVersion, fileHeader.minorVersion);
			continue;
		}

		// We should be sane now...
		fprintf(stdout, "EXP FILE %s\n", fileName);
		fprintf(stdout, "Version:  %d.%d\n", fileHeader.majorVersion, fileHeader.minorVersion);
		fprintf(stdout, "Sections: %d\n", fileHeader.numSections);
		fprintf(stdout, "Metas:    %d\n", fileHeader.numMeta);

		if (dumpSections)
		{
			fprintf(stdout, "Sections:\n");

			for (int i = 0; i < fileHeader.numSections; i++)
			{
				expsection_t section;

				fseek(f, fileHeader.posSections + i * sizeof(expsection_t), SEEK_SET);
				fread(&section, 1, sizeof(expsection_t), f);

				fprintf(stdout, " Section #%d\n", i);
				fprintf(stdout, "  Name:  %s\n", section.name);
				fprintf(stdout, "  Type:  %s\n", (section.type ? "Data" : "Code"));
				fprintf(stdout, "  Base:  0x%X\n", section.base);
				fprintf(stdout, "  Start: %d\n", section.start);
				fprintf(stdout, "  Size:  %d\n", section.length);
				
				// Call disassembler
				if (disassembleSections && section.type == 0)
				{
					fprintf(stdout, "  Disassembly:\n");

					Instruction *buffer = malloc(section.length);

					fseek(f, section.start, SEEK_SET);
					int len = fread(buffer, 1, section.length, f);
					if (len != section.length)
						fprintf(stderr, "Read invalid size.\n");

					disassemble(
						buffer,
						section.length / sizeof(Instruction),
						section.base,
						stdout);

					free(buffer);
				}
			}
		}

		///////////////////////////////////////////////////////////////////////////////

		fclose(f);
	}
	return 0;
}