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

void disassemble(Instruction *list, uint32_t count, FILE *f)
{
	for (int i = 0; i < count; i++)
	{
		Instruction instr = list[i];

		fprintf(f, "\t");

		fprintf(f, "%6x  ", i);

		switch (instr.execN)
		{
		case VM_EXEC_0: fprintf(f, "[ex(n)=0] "); break;
		case VM_EXEC_1: fprintf(f, "[ex(n)=1] "); break;
		case VM_EXEC_X: fprintf(f, "[ex(n)=x] "); break;
		}

		switch (instr.execZ)
		{
		case VM_EXEC_0: fprintf(f, "[ex(z)=0] "); break;
		case VM_EXEC_1: fprintf(f, "[ex(z)=1] "); break;
		case VM_EXEC_X: fprintf(f, "[ex(z)=x] "); break;
		}

		switch (instr.input0)
		{
		case VM_INPUT_ZERO: fprintf(f, "[i0:zero] "); break;
		case VM_INPUT_POP: fprintf(f, "[i0:pop] "); break;
		case VM_INPUT_PEEK: fprintf(f, "[i0:peek] "); break;
		case VM_INPUT_ARG: fprintf(f, "[i0:arg] "); break;
		}

		switch (instr.input1)
		{
		case VM_INPUT_ZERO: fprintf(f, "[i1:zero] "); break;
		case VM_INPUT_POP: fprintf(f, "[i1:pop] "); break;
		case VM_INPUT_PEEK: fprintf(f, "[i1:peek] "); break;
		case VM_INPUT_ARG: fprintf(f, "[i1:arg] "); break;
		}

		if (instr.command <= 12)
			fprintf(f, "%s", commandStrings[instr.command]);
		else
			fprintf(f, "undefined [cmd:%d]", instr.command);

		fprintf(f, " [ci:%d]", instr.cmdinfo);

		switch (instr.flags)
		{
		case VM_FLAG_NO: fprintf(f, " [f:no]"); break;
		case VM_FLAG_YES: fprintf(f, " [f:yes]"); break;
		}

		switch (instr.output)
		{
		case VM_OUTPUT_DISCARD: fprintf(f, " [r:discard]"); break;
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
	while ((c = getopt(argc, argv, "smd")) != -1)
	{
		switch (c)
		{
		case 's': dumpSections = 1; break;
		case 'm': dumpMetas = 1; break;
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