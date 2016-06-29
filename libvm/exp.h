#pragma once

#if defined(__gcc)
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif

#include <stdint.h>

static const uint32_t EXP_MAGIC = 0x00505845; // 'E' 'X' 'P' '\0'

struct expfile
{
	uint32_t magicNumber;  // MUST BE EXP_MAGIC
	uint16_t majorVersion; // 1
	uint16_t minorVersion; // 0
	uint32_t numMeta;      // Number of metadata entries
	uint32_t numSections;  // Number of sections
	uint32_t posMeta;      // File pointer of first metadata entry
	uint32_t posSections;  // File pointer of first section definition;
} PACKED;

struct expsection
{
	uint32_t type;     // Type of the section: 0 = code, 1 = data
	uint32_t base;     // Base address of the section in memory.
	uint32_t start;    // File pointer to the begin of the section
	uint32_t length;   // Length of the section in bytes
	char     name[64]; // Name of the section, null terminated c-string
} PACKED;

struct expstring
{
	uint32_t start;    // File pointer to the start of the string
	uint32_t length;   // Length of the string in bytes.
} PACKED;

struct expmeta
{
	uint32_t type; // Type of the metadata: 0 = uint32_t, 1 = int32_t, 2 = ExpString
	char key[32];  // Name of the metadata, null terminated c-string
	union {
		uint32_t  u; // uint32_t
		int32_t   i; // int32_t
		struct expstring s; // ExpString
	} value;       // Value of the metadata
} PACKED;

typedef struct expfile expfile_t;
typedef struct expsection expsection_t;
typedef struct expstring expstring_t;
typedef struct expmeta expmeta_t;