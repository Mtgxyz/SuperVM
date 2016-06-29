#include <stdint.h>
#include <stddef.h>

const uint32_t EXP_MAGIC = 0x00505845; // 'E' 'X' 'P' '\0'

struct expfile
{
	uint32_t magicNumber;  // MUST BE EXP_MAGIC
	uint16_t majorVersion; // 1
	uint16_t minorVersion; // 0
	uint32_t numMeta;      // Number of metadata entries
	uint32_t numSections;  // Number of sections
	uint32_t posMeta;      // File pointer of first metadata entry
	uint32_t posSections;  // File pointer of first section definition;
} __attribute__ ((packed));

struct expsection
{
	uint32_t type;     // Type of the section: 0 = code, 1 = data
	uint32_t start;    // File pointer to the begin of the section
	uint32_t length;   // Length of the section in bytes
	char     name[64]; // Name of the section, null terminated c-string
} __attribute__ ((packed));

struct expstring
{
	uint32_t start;    // File pointer to the start of the string
	uint32_t length;   // Length of the string in bytes.
} __attribute__ ((packed));

struct expmeta
{
	uint32_t type; // Type of the metadata: 0 = uint32_t, 1 = int32_t, 2 = ExpString
	char key[32];  // Name of the metadata, null terminated c-string
	union {
		uint32_t  u; // uint32_t
		int32_t   i; // int32_t
		struct expstring s; // ExpString
	} value;       // Value of the metadata
} __attribute__ ((packed));

typedef struct expfile expfile_t;
typedef struct expsection expsection_t;
typedef struct expstring expstring_t;
typedef struct expmeta expmeta_t;

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>

#define DEBUG_VAL(x) fprintf(stderr, #x " = %d\n", x)

int main(int argc, char **argv)
{
	char const *outfileName = "a.exp";
  char const *codefileName = NULL;
  char const *datafileName = NULL;
	int memsize = 65536;
  opterr = 0;
	int c;
  while ((c = getopt (argc, argv, "c:d:m:o:")) != -1)
	{
    switch (c)
		{
		case 'm':
			memsize = atoi(optarg);
			if(memsize <= 0) {
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
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
			return 1;
		default:
			abort();
		}
	}
  for (int index = optind; index < argc; index++)
    printf ("Non-option argument %s\n", argv[index]);
	
	FILE *f = fopen(outfileName, "wb");
	if(f == NULL)
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
	if(codefileName != NULL) numSections++;
	if(datafileName != NULL) numSections++;

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
	
	expsection_t codeSection = { 0 };
	strcpy(codeSection.name, ".code");
	
	expsection_t dataSection = { 1 };
	strcpy(dataSection.name, ".data");
	
	if(codefileName != NULL)
		fwrite(&codeSection, sizeof(expsection_t), 1, f);
	if(datafileName != NULL)
		fwrite(&dataSection, sizeof(expsection_t), 1, f);
	
	if(codefileName != NULL)
	{
		codeSection.start = ftell(f);
		
		FILE *fc = fopen(codefileName, "rb");
		if(fc == NULL)
		{
			fprintf(stderr, "Could not open %s\n", codefileName);
			fclose(f);
			return 1;
		}
		
		char buffer[4096];
		while(!feof(fc))
		{
			int len = fread(buffer, 1, 4096, fc);
			if(len > 0)
				fwrite(buffer, 1, len, f);
			codeSection.length += len;
		}
		
		fclose(fc);
		
		DEBUG_VAL(codeSection.start);
		DEBUG_VAL(codeSection.length);
	}
	if(datafileName != NULL)
	{
		dataSection.start = ftell(f);
		
		FILE *fc = fopen(datafileName, "rb");
		if(fc == NULL)
		{
			fprintf(stderr, "Could not open %s\n", datafileName);
			fclose(f);
			return 1;
		}
		
		char buffer[4096];
		while(!feof(fc))
		{
			int len = fread(buffer, 1, 4096, fc);
			if(len > 0)
				fwrite(buffer, 1, len, f);
			dataSection.length += len;
		}
		
		fclose(fc);
		
		DEBUG_VAL(dataSection.start);
		DEBUG_VAL(dataSection.length);
	}
	
	
	// Now write the header again...
	rewind(f);
	fwrite(&fileHeader, sizeof(expfile_t), 1, f);
	
	fseek(f, fileHeader.posSections, SEEK_SET);
	if(codefileName != NULL)
		fwrite(&codeSection, sizeof(expsection_t), 1, f);
	if(datafileName != NULL)
		fwrite(&dataSection, sizeof(expsection_t), 1, f);
	
///////////////////////////////////////////////////////////////////////////////
	
	
	
	return 0;
}