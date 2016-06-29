#pragma once

#include <stddef.h>
#include <inttypes.h>

#if defined(__gcc)
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif


#if defined(__cplusplus)
extern "C" {
#else
#if defined(_gcc)
#define static_assert  _Static_assert 
#else
#define static_assert(x,y)
#endif
#endif

#if !defined(VM_STACKSIZE)
#define VM_STACKSIZE 512
#endif

// Binary Encoding : (enabled, value)
#define VM_EXEC_X 0
#define VM_EXEC_0 2
#define VM_EXEC_1 3

#define VM_INPUT_ZERO 0
#define VM_INPUT_POP 1
#define VM_INPUT_PEEK 2
#define VM_INPUT_ARG 3

#define VM_CMD_COPY     0
#define VM_CMD_STORE    1
#define VM_CMD_LOAD     2  
#define VM_CMD_GET      3
#define VM_CMD_SET      4
#define VM_CMD_BPGET    5
#define VM_CMD_BPSET    6
#define VM_CMD_CPGET    7
#define VM_CMD_MATH     8
#define VM_CMD_SPGET    9
#define VM_CMD_SPSET    10
#define VM_CMD_SYSCALL  11
#define VM_CMD_HWIO     12

#define VM_MATH_ADD 0
#define VM_MATH_SUB 1
#define VM_MATH_MUL 2
#define VM_MATH_DIV 3
#define VM_MATH_MOD 4
#define VM_MATH_AND 5
#define VM_MATH_OR  6
#define VM_MATH_XOR 7
#define VM_MATH_NOT 8
#define VM_MATH_ROL 9
#define VM_MATH_ROR 10
#define VM_MATH_ASL 11
#define VM_MATH_ASR 12
#define VM_MATH_SHL 13
#define VM_MATH_SHR 14

#define VM_FLAG_NO  0
#define VM_FLAG_YES 1

#define VM_OUTPUT_DISCARD 0
#define VM_OUTPUT_PUSH    1
#define VM_OUTPUT_JUMP    2
#define VM_OUTPUT_JUMPR   3

#define VM_FLAG_Z (1<<0)
#define VM_FLAG_N (1<<1)

struct instruction
{
	unsigned int execZ : 2;
	unsigned int execN : 2;
	unsigned int input0 : 2;
	unsigned int input1 : 1;
	unsigned int command : 6;
	unsigned int cmdinfo : 16;
	unsigned int flags : 1;
	unsigned int output : 2;
	uint32_t     argument;
} PACKED ;

static_assert(sizeof(Instruction) == 8, "Instruction must be 8 bytes large.");
static_assert(offsetof(Instruction, argument) == 4, "Argument must be  must be 8 bytes large.");

struct spu
{
	struct instruction *code;            // Pointer to the first instruction
	uint32_t codeLength;          // length of the code in instructions

	uint32_t codePointer;         // code pointer register
	uint32_t stackPointer;        // stack pointer register
	uint32_t basePointer;         // base pointer register
	uint32_t flags;               // flag register
	
	uint32_t memoryBase;          // Linear start address of the memory.
	void *memory;                 // Point to the base linear address
	uint32_t memorySize;          // Size of the memory in bytes.

	uint32_t stack[VM_STACKSIZE]; // The stack of the SPU.
};

struct cmdinput
{
	uint32_t input0; // first input argument
	uint32_t input1; // second input argument
	uint32_t info;   // command info
};

typedef struct instruction instruction_t;
typedef struct spu spu_t;
typedef struct cmdinput cmdinput_t;


/**
* @brief Steps a given process.
*
* Executes a single instruction and processes input and output.
*
* @param process The process to be stepped.
* @returns 1 if the process is still running or 0 if the process is terminated.
*/
int vm_step_process(spu_t *process);

/**
* @brief Pushes a value onto the process' stack.
*/
void vm_push(spu_t *process, uint32_t value);

/**
* @brief Pops a value from the process' stack.
*/
uint32_t vm_pop(spu_t *process);

/**
* @brief Returns the top value of the process' stack.
*/
uint32_t vm_peek(spu_t *process);

/**
* Reads a byte from process memory.
* @arg process
* @arg address The address to read from.
*/
uint8_t vm_read_byte(spu_t *process, uint32_t address);

/**
* Writes a byte to process memory.
* @arg process
* @arg address The address to read from.
*/
void vm_write_byte(spu_t *process, uint32_t address, uint8_t value);

// The following functions need to be host-implemented.

/**
* An assertion the VM does.
* @param assertion If zero, the assertion failed.
* @param msg       The message that should be shown when the assertion fails.
*/
void vm_assert(int assertion, const char *msg);

/**
* The hosts syscall implementation.
* @param process The process that calls the syscall.
* @param info    The input values of the syscall.
* @returns       Result the SPU receives from this command.
*/
uint32_t vm_syscall(spu_t *process, cmdinput_t *info);

/**
* The hosts hardware IO implementation.
* @param process The process that wants to do IO.
* @param info    The input values for the hardware IO.
* @returns       Result the SPU receives from this command.
*/
uint32_t vm_hwio(spu_t *process, cmdinput_t *info);

#if defined(__cplusplus)
}
#else
#undef static_assert
#endif