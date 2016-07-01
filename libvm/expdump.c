#if !defined(__gcc)
#define  _CRT_SECURE_NO_WARNINGS
#endif

#include <stdint.h>
#include <stddef.h>

#include "exp.h"
#include "vm.h"
#include "disassembler.h"

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

int main(int argc, char **argv)
{
	opterr = 0;

	int headers = 0;
	int dumpSections = 0;
	int dumpMetas = 0;
	int disassembleSections = 0;

	int c;
	while ((c = getopt(argc, argv, "HsmdD")) != -1)
	{
		switch (c)
		{
		case 'H': headers = 1; break;
		case 's': dumpSections = 1; break;
		case 'm': dumpMetas = 1; break;
		case 'D': disasmOptions.verbose = true;
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
	
	if(!headers && !dumpSections && !dumpMetas && !disassembleSections) {
		headers = 1;
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
			fprintf(stderr, "Invalid magic in %s\n", fileName);
			continue;
		}
		if (fileHeader.majorVersion != 1 && fileHeader.minorVersion == 0)
		{
			fprintf(
				stderr, "Invalid version %s: %d.%d\n",
				fileName,
				fileHeader.majorVersion, fileHeader.minorVersion);
			continue;
		}

		if(headers)
		{
			// We should be sane now...
			fprintf(stdout, "EXP FILE %s\n", fileName);
			fprintf(stdout, "Version:  %d.%d\n", fileHeader.majorVersion, fileHeader.minorVersion);
			fprintf(stdout, "Sections: %d\n", fileHeader.numSections);
			fprintf(stdout, "Metas:    %d\n", fileHeader.numMeta);
		}
		
		if (dumpSections || disassembleSections)
		{
			if(dumpSections) fprintf(stdout, "Sections:\n");

			for (int i = 0; i < fileHeader.numSections; i++)
			{
				expsection_t section;

				fseek(f, fileHeader.posSections + i * sizeof(expsection_t), SEEK_SET);
				fread(&section, 1, sizeof(expsection_t), f);
				if(dumpSections)
				{
					fprintf(stdout, " Section #%d\n", i);
					fprintf(stdout, "  Name:  %s\n", section.name);
					fprintf(stdout, "  Type:  %s\n", (section.type ? "Data" : "Code"));
					fprintf(stdout, "  Base:  0x%X\n", section.base);
					fprintf(stdout, "  Start: %d\n", section.start);
					fprintf(stdout, "  Size:  %d\n", section.length);
				}	
				// Call disassembler
				if (disassembleSections && section.type == 0)
				{
					if(dumpSections)
						fprintf(stdout, "  Disassembly:\n");
					else
						fprintf(stdout, "; Section '%s'@0x%08X\n", section.name, section.base);

					instruction_t *buffer = malloc(section.length);

					fseek(f, section.start, SEEK_SET);
					int len = fread(buffer, 1, section.length, f);
					if (len != section.length)
						fprintf(stderr, "Read invalid size.\n");

					disassemble(
						buffer,
						section.length / sizeof(instruction_t),
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
