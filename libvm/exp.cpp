#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <getopt.h>

#include <iostream>
#define DEBUG_VAL(x) std::cout << #x << " = " << (x) << "\n";

#include "include/supervm/exp.hpp"
#include "include/streams.hpp"

int main(int argc, char **argv)
{
  char const *outfileName = "a.exp";
  char const *codefileName = nullptr;
  char const *datafileName = nullptr;
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
			if (optopt == 'o')
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


	FileStream fs(outfileName, "wb");
	
	exp::File fileHeader = {
		.magicNumber = EXP_MAGIC,
		.majorVersion = 1,
		.minorVersion = 0,
		.numMeta = 0,
		.numSections = 2,
		.posMeta = 0,
		.posSections = 0,
	};
	
	fileHeader.posSections = fs.write(&fileHeader, sizeof(exp::File));
	
	DEBUG_VAL(fileHeader.posSections);
	
	exp::Section codeSection = {
		.type = 0,
		.start = 0,
		.length = 26,
	};
	strcpy(codeSection.name, ".code");
	
	exp::Section dataSection = {
		.type = 1,
		.start = 0,
		.length = 10,
	};
	strcpy(dataSection.name, ".data");
	
	fs.write(&codeSection, sizeof(exp::Section));
	fs.write(&dataSection, sizeof(exp::Section));
	
	codeSection.start = fs.write("abcdefghijklmnopqrstuvwxyz", 26);
	dataSection.start = fs.write("0123456789", 10);
	
	DEBUG_VAL(codeSection.start);
	DEBUG_VAL(dataSection.start);
	
	// Now write the header again...
	fs.rewind();
	fs.write(&fileHeader, sizeof(exp::File));
	
	fs.seek(SeekMode::Start, fileHeader.posSections);
	fs.write(&codeSection, sizeof(exp::Section));
	fs.write(&dataSection, sizeof(exp::Section));
	
  return 0;
}
