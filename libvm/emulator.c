#define _CRT_SECURE_NO_WARNINGS

#include <SDL.h>
#include <stdbool.h>
#include "vm.h"
#include "exp.h"

#if defined(_MSC_VER)
#undef main
#endif

bool running = true;

/**
* An assertion the VM does.
* @param assertion If zero, the assertion failed.
* @param msg       The message that should be shown when the assertion fails.
*/
void vm_assert(int assertion, const char *msg)
{
	if (assertion)
		return;
	printf("Assertion failed: %s\n", msg);
	running = false;
}

/**
* The hosts syscall implementation.
* @param process The process that calls the syscall.
* @param info    Additional information for the syscall. Contains arguments and results.
*/
uint32_t vm_syscall(spu_t *process, cmdinput_t *info)
{
	fprintf(stdout, "SYSCALL [%d]: (%d, %d)\n", info->info, info->input0, info->input1);
	if (info->info == 0)
		running = false;
	return 0;
}

/**
* The hosts hardware IO implementation.
* @param process The process that wants to do IO.
* @param info    Additional information for the HWIO. Contains arguments and results.
*/
uint32_t vm_hwio(spu_t *process, cmdinput_t *info)
{
	return 0;
}

spu_t mainCore;

void dump_vm()
{
	printf(
		"%8X %3d %3d %1X [",
		mainCore.codePointer,
		mainCore.stackPointer,
		mainCore.basePointer,
		mainCore.flags
	);

	for (int i = 1; i < mainCore.stackPointer; i++)
	{
		printf(" %d", mainCore.stack[i]);
	}

	printf(" ]\n");
}

void update_vm()
{
	vm_step_process(&mainCore);

	// dump_vm();
}

void update_input(SDL_Event *ev)
{
	switch (ev->type)
	{
	case SDL_QUIT: 
		running = false;
		break;
	}
}

void initialize_vm()
{
	// Initialize memory
	mainCore.memoryBase = 0x00; // Linear memory, start at 0x00
	mainCore.memorySize = 0x4000000; // 64 MB;
	mainCore.memory = malloc(mainCore.memorySize);

	// Initialize code execution
	mainCore.code = mainCore.memory;
	mainCore.codeLength = mainCore.memorySize / sizeof(instruction_t);
	
	// Setup registers
	mainCore.codePointer = 0;
	mainCore.stackPointer = 0;
	mainCore.basePointer = 0;
	mainCore.flags = 0;

	// Clear stack
	memset(mainCore.stack, 0, VM_STACKSIZE * sizeof(uint32_t));
}

bool load_exp(const char *fileName)
{
	FILE *f = fopen(fileName, "rb");
	if (f == NULL)
	{
		fprintf(stderr, "Could not open file %s\n", fileName);
		return false;
	}

	///////////////////////////////////////////////////////////////////////////////

	expfile_t fileHeader;

	if (fread(&fileHeader, 1, sizeof(expfile_t), f) != sizeof(expfile_t))
	{
		fprintf(stderr, "File %s does not contain a valid header.\n", fileName);
		return false;
	}
	if (fileHeader.magicNumber != EXP_MAGIC)
	{
		fprintf(stderr, "Invalid magic in %s\n", fileName);
		return false;
	}
	if (fileHeader.majorVersion != 1 && fileHeader.minorVersion == 0)
	{
		fprintf(
			stderr, "Invalid version %s: %d.%d\n",
			fileName,
			fileHeader.majorVersion, fileHeader.minorVersion);
		return false;
	}

	for (uint32_t i = 0; i < fileHeader.numSections; i++)
	{
		expsection_t section;

		fseek(f, fileHeader.posSections + i * sizeof(expsection_t), SEEK_SET);
		fread(&section, 1, sizeof(expsection_t), f);
		
		fseek(f, section.start, SEEK_SET);

		uint8_t *sectionInRam = (uint8_t*)mainCore.memory + section.base;
		int len = fread(sectionInRam, 1, section.length, f);
		if (len != section.length)
			fprintf(stderr, "Read invalid size.\n");
	}

	return true;
}

int main(int argc, char **argv)
{
	// Required before ANY virtual machine memory operations...
	initialize_vm();

	// Load the EXP file
	if (argc > 1)
	{
		if (!load_exp(argv[1])) {
			return 1;
		}
	}

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		return 1;
	}
	atexit(SDL_Quit);

	SDL_Surface *screen = SDL_SetVideoMode(640, 480, 32, SDL_DOUBLEBUF);
	SDL_WM_SetCaption("DasOS Virtual Platform", NULL);

	dump_vm();

	SDL_Event ev;
	while (running)
	{
		while (SDL_PollEvent(&ev))
		{
			update_input(&ev);
		}

		uint32_t start = SDL_GetTicks();

		do {
			for (int i = 0; i < 50 && running; i++)
			{
				update_vm();
			}
		} while (running && (SDL_GetTicks() - start) <= 32);

		{ // copy screen
			SDL_LockSurface(screen);
			memcpy(
				screen->pixels,
				(uint8_t*)mainCore.memory + 4096,
				screen->h * screen->pitch);
			SDL_UnlockSurface(screen);
		}

		SDL_Flip(screen);
	}

	SDL_WM_SetCaption("DasOS Virtual Platform - STOPPED", NULL);

	running = true;
	while (running)
	{
		while (SDL_PollEvent(&ev))
		{
			if (ev.type == SDL_QUIT)
				running = false;
			else if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
				running = false;
		}

		SDL_Flip(screen);

		SDL_Delay(32);
	}
	
	return 0;
}

FILE * __cdecl __iob_func(void)
{
	static FILE iob[3];
	iob[0] = *stdin;
	iob[1] = *stdout;
	iob[2] = *stderr;
	return iob;
}