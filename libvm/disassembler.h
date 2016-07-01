#pragma once

#include "vm.h"
#include <stdio.h>
#include <stdbool.h>

struct disassembler_options
{
	bool verbose;
	bool outputAddresses;
};

// should the disassembler be more verbose?
extern struct disassembler_options disasmOptions;

// Disassembles a given list of instructions
void disassemble(
	instruction_t *list, // Array with
	uint32_t count,      // count instructions.
	uint32_t base,       // The base instruction offset, "entry point"
	FILE *f              // The file where the disassembly should be printed.
);           