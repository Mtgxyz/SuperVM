#if !defined(__gcc)
#define  _CRT_SECURE_NO_WARNINGS
#endif

#include <stdint.h>
#include <stddef.h>

#include "exp.h"


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
	char const *outfileName = "a.exp";
	char const *codefileName = NULL;
	char const *datafileName = NULL;
	int memsize = 65536;
	int codeStart = 0;
	int dataStart = 0;
	opterr = 0;
	int c;
	while ((c = getopt(argc, argv, "C:D:c:d:m:o:")) != -1)
	{
		switch (c)
		{
		case 'C':
			codeStart = atoi(optarg);
			break;
		case 'D':
			dataStart = atoi(optarg);
			break;
		case 'm':
			memsize = atoi(optarg);
			if (memsize <= 0) {
				fprintf(stderr, "memsize must be larger 0.\n");
				return 1;
			}
		case 'o':
			outfileName = optarg;
			break;
		case 'c':
			codefileName = optarg;
			break;
		case 'd':
			datafileName = optarg;
			break;
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
		printf("Non-option argument %s\n", argv[index]);

	FILE *f = fopen(outfileName, "wb");
	if (f == NULL)
	{
		fprintf(stderr, "Could not open file %s\n", outfileName);
		return 1;
	}

	///////////////////////////////////////////////////////////////////////////////

	fprintf(stderr, "Out:  %s\n", outfileName);
	fprintf(stderr, "Code: %s\n", codefileName);
	fprintf(stderr, "Data: %s\n", datafileName);

	///////////////////////////////////////////////////////////////////////////////

	int numSections = 0;
	if (codefileName != NULL) numSections++;
	if (datafileName != NULL) numSections++;

	///////////////////////////////////////////////////////////////////////////////

	expfile_t fileHeader = {
		EXP_MAGIC,
		1, 0, // version
		0, numSections, // n(meta), n(sections)
		0, 0, // pos(meta), pos(sections)
	};

	fwrite(&fileHeader, sizeof(expfile_t), 1, f);
	fileHeader.posSections = ftell(f);

	DEBUG_VAL(fileHeader.posSections);

	expsection_t codeSection = { 0, codeStart };
	strcpy(codeSection.name, ".code");

	expsection_t dataSection = { 1, dataStart };
	strcpy(dataSection.name, ".data");

	DEBUG_VAL(codeSection.base);
	DEBUG_VAL(dataSection.base);

	if (codefileName != NULL)
		fwrite(&codeSection, sizeof(expsection_t), 1, f);
	if (datafileName != NULL)
		fwrite(&dataSection, sizeof(expsection_t), 1, f);

	if (codefileName != NULL)
	{
		codeSection.start = ftell(f);

		FILE *fc = fopen(codefileName, "rb");
		if (fc == NULL)
		{
			fprintf(stderr, "Could not open %s\n", codefileName);
			fclose(f);
			return 1;
		}

		char buffer[4096];
		while (!feof(fc))
		{
			size_t len = fread(buffer, 1, 4096, fc);
			if (len > 0)
				fwrite(buffer, 1, len, f);
			codeSection.length += (uint32_t)len;
		}

		fclose(fc);

		DEBUG_VAL(codeSection.start);
		DEBUG_VAL(codeSection.length);
	}
	if (datafileName != NULL)
	{
		dataSection.start = ftell(f);

		FILE *fc = fopen(datafileName, "rb");
		if (fc == NULL)
		{
			fprintf(stderr, "Could not open %s\n", datafileName);
			fclose(f);
			return 1;
		}

		char buffer[4096];
		while (!feof(fc))
		{
			size_t len = fread(buffer, 1, 4096, fc);
			if (len > 0)
				fwrite(buffer, 1, len, f);
			dataSection.length += (uint32_t)len;
		}

		fclose(fc);

		DEBUG_VAL(dataSection.start);
		DEBUG_VAL(dataSection.length);
	}


	// Now write the header again...
	rewind(f);
	fwrite(&fileHeader, sizeof(expfile_t), 1, f);

	fseek(f, fileHeader.posSections, SEEK_SET);
	if (codefileName != NULL)
		fwrite(&codeSection, sizeof(expsection_t), 1, f);
	if (datafileName != NULL)
		fwrite(&dataSection, sizeof(expsection_t), 1, f);

	///////////////////////////////////////////////////////////////////////////////

	return 0;
}