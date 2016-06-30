#include "vm.h"

#include <stdio.h>

static uint32_t cmd_copy(cmdinput_t *info)
{
	return info->input0;
}

static uint32_t cmd_load(spu_t *p, cmdinput_t *info)
{
	uint32_t result = 0;
	switch(info->info) {
		case 2:
			result |=(uint32_t)(vm_read_byte(p, info->input0+3))<<24;
			result |=(uint32_t)(vm_read_byte(p, info->input0+2))<<16;
		case 1:
			result |=(uint32_t)(vm_read_byte(p, info->input0+1))<< 8;
		case 0:
			result |=(uint32_t)(vm_read_byte(p, info->input0+0))<< 0;
			break;
	}
	return result;
}

static uint32_t cmd_store(spu_t *p, cmdinput_t *info)
{
	switch(info->info) {
		case 2:
			vm_write_byte(p, info->input0+3, info->input1>>24);
			vm_write_byte(p, info->input0+2, info->input1>>16);
		case 1:
			vm_write_byte(p, info->input0+1, info->input1>>8);
		case 0:
			vm_write_byte(p, info->input0, info->input1);
			break;
	}
	return info->input1;
}

static uint32_t cmd_spget(spu_t *p, cmdinput_t *info)
{
	return p->stackPointer;
}

static uint32_t cmd_spset(spu_t *p, cmdinput_t *info)
{
	return p->stackPointer = info->input0;
}

static uint32_t cmd_bpget(spu_t *p, cmdinput_t *info)
{
	return  p->basePointer;
}

static uint32_t cmd_bpset(spu_t *p, cmdinput_t *info)
{
	return p->basePointer = info->input0;
}

static uint32_t cmd_cpget(spu_t *p, cmdinput_t *info)
{
	return p->codePointer + info->info;
}

static inline int16_t makeSigned(uint16_t val)
{
	return *((int16_t*)&val);
}

static uint32_t cmd_get(spu_t *p, cmdinput_t *info)
{
	return p->stack[p->basePointer + makeSigned(info->input0)];
}

static uint32_t cmd_set(spu_t *p, cmdinput_t *info)
{
	return p->stack[p->basePointer + makeSigned(info->input0)] = info->input1;
}

static uint32_t cmd_math(cmdinput_t *info)
{
	switch(info->info)
	{
		// IMPORTANT:
		// input1 - input0 because then input0 can be a fixed value 
#define S(name, op) case name: return info->input1 op info->input0; break;
		S(VM_MATH_ADD, +)
		S(VM_MATH_SUB, -)
		S(VM_MATH_MUL, *)
		S(VM_MATH_DIV, /)
		S(VM_MATH_MOD, %)
		S(VM_MATH_AND, &)
		S(VM_MATH_OR, |)
		S(VM_MATH_XOR, ^)
#undef S
		case VM_MATH_NOT: return ~info->input0; break;
		default: vm_assert(0, "Invalid instruction: MATH command not defined."); return -1;
	}
}

int vm_step_process(spu_t *process)
{
	vm_assert(process != NULL, "process must not be NULL.");
	instruction_t instr = process->code[process->codePointer++];

	int exec = 1;
	switch(instr.execZ)
	{
		case VM_EXEC_X:
			/* Don't modify execution. */
			break;
		case VM_EXEC_0:
			if((process->flags & VM_FLAG_Z) != 0)
				exec = 0;
			break;
		case VM_EXEC_1:
			if((process->flags & VM_FLAG_Z) == 0)
				exec = 0;
			break;
		default:
			vm_assert(0, "Invalid instruction: execZ undefined.");
			break;
	}
	
	switch(instr.execN)
	{
		case VM_EXEC_X:
			/* Don't modify execution. */
			break;
		case VM_EXEC_0:
			if((process->flags & VM_FLAG_N) != 0)
				exec = 0;
			break;
		case VM_EXEC_1:
			if((process->flags & VM_FLAG_N) == 0)
				exec = 0;
			break;
		default:
			vm_assert(0, "Invalid instruction: execN undefined.");
			break;
	}
	
	// Only do further instruction execution when
	// the execution condition is met.
	if(exec)
	{
		cmdinput_t info = { 0 };
		switch(instr.input0)
		{
			case VM_INPUT_ZERO: info.input0 = 0; break;
			case VM_INPUT_POP:  info.input0 = vm_pop(process); break;
			case VM_INPUT_PEEK: info.input0 = vm_peek(process); break;
			case VM_INPUT_ARG:  info.input0 = instr.argument; break;
			default: vm_assert(0, "Invalid instruction: input0 undefined.");
		}
		
		switch(instr.input1)
		{
			case VM_INPUT_ZERO: info.input1 = 0; break;
			case VM_INPUT_POP:  info.input1 = vm_pop(process); break;
			default: vm_assert(0, "Invalid instruction: input1 undefined.");
		}
		
		info.info = instr.cmdinfo;
		
		uint32_t output = -1;
		switch(instr.command)
		{
			case VM_CMD_COPY: output = cmd_copy(&info); break;
			case VM_CMD_STORE: output = cmd_store(process, &info); break;
			case VM_CMD_LOAD: output = cmd_load(process, &info); break;
			case VM_CMD_MATH: output = cmd_math(&info); break;
			case VM_CMD_SYSCALL: output = vm_syscall(process, &info); break;
			case VM_CMD_HWIO: output = vm_hwio(process, &info); break;
			case VM_CMD_SPGET: output = cmd_spget(process, &info); break;
			case VM_CMD_SPSET: output = cmd_spset(process, &info); break;
			case VM_CMD_BPGET: output = cmd_bpget(process, &info); break;
			case VM_CMD_BPSET: output = cmd_bpset(process, &info); break;
			case VM_CMD_CPGET: output = cmd_cpget(process, &info); break;
			case VM_CMD_GET: output = cmd_get(process, &info); break;
			case VM_CMD_SET: output = cmd_set(process, &info); break;
			default: vm_assert(0, "Invalid instruction: command undefined.");
		}
		
		switch(instr.flags)
		{
			case VM_FLAG_YES:
				process->flags = 0;
				if(output == 0)
					process->flags |= VM_FLAG_Z;
				else if((output & (1<<31)) != 0)
					process->flags |= VM_FLAG_N;
				break;
			case VM_FLAG_NO: break;
			default:
				vm_assert(0, "Invalid instruction: invalid flags.");
		}
		
		switch(instr.output)
		{
			case VM_OUTPUT_DISCARD: break;
			case VM_OUTPUT_PUSH: vm_push(process, output); break;
			case VM_OUTPUT_JUMP: process->codePointer = output; break;
			case VM_OUTPUT_JUMPR:
				process->codePointer += *((int32_t*)&output); 
				break;
			default:
				vm_assert(0, "Invalid instruction: invalid output.");
		}
	}
	
	return process->codePointer < process->codeLength;
}

void vm_push(spu_t *process, uint32_t value)
{
	vm_assert(process != NULL, "process must not be NULL.");
	vm_assert(process->stackPointer < VM_STACKSIZE, "Stack overflow");
	process->stack[++process->stackPointer] = value;
}

uint32_t vm_pop(spu_t *process)
{
	vm_assert(process != NULL, "process must not be NULL.");
	uint32_t psp = process->stackPointer;
	uint32_t val = process->stack[process->stackPointer--];
	
	// Underflow check works because unsigned overflow is defined ;)
	vm_assert(psp >= process->stackPointer, "Stack underflow");
	
	return val;
}

uint32_t vm_peek(spu_t *process)
{
	vm_assert(process != NULL, "process must not be NULL.");
	return process->stack[process->stackPointer];
}





uint8_t vm_read_byte(spu_t *process, uint32_t address)
{
	vm_assert(process != NULL, "process must not be NULL.");
	address -= process->memoryBase;
	vm_assert(address < process->memorySize, "Out of memory.");
	return ((uint8_t*)process->memory)[address];
}

void vm_write_byte(spu_t *process, uint32_t address, uint8_t value)
{
	vm_assert(process != NULL, "process must not be NULL.");
	address -= process->memoryBase;
	vm_assert(address < process->memorySize, "Out of memory.");
	((uint8_t*)process->memory)[address] = value;
}
