
#include <stdbool.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "tokens.h"
#include "../vm.h"
#include "../mnemonics.h"

struct llist
{
	char name[128];
	uint32_t value;
	struct llist *next;
};

typedef struct llist llist_t;

void list_insert(llist_t **list, const char *name, uint32_t value)
{
	llist_t *item = malloc(sizeof(struct llist));
	strcpy(item->name, name);
	item->value = value;
	item->next = *list;
	*list = item;
}

uint32_t list_find(llist_t **list, const char *name)
{
	for(llist_t *it = *list; it != NULL; it = it->next)
	{
		if(strcmp(it->name, name) == 0) {
			return it->value;
		}
	}
	return -1;
}

void list_free(llist_t **list)
{
	while((*list) != NULL)
	{
		llist_t *next = (*list)->next;
		free(*list);
		(*list) = next;
	}
}

FILE *output;

int listSymbols = 0;
int entryPoint = 0;

// current token
int tok;

void apply_modifier(instruction_t *i, const char *mod);

void apply_modifiers(instruction_t *i)
{
	while(tok == TOK_MOD)
	{
		char *s = yytext + 1;
		int len = strlen(s);
		s[len - 1] = 0;
		apply_modifier(i, s);
		tok = yylex();
	}
}

void compile_error(const char *msg, ...)
{
	fprintf(stderr, "test.asm:%d:", yylineno);
	
  va_list args;
  va_start (args, msg);
  vfprintf (stderr, msg, args);
  va_end (args);
	
	exit(1);
}

static llist_t *labels = NULL;
static llist_t *patches = NULL;

// line format:
// TOK_LABEL? TOK_MOD* TOK_MNEMONIC TOK_MOD* (TOK_INT|TOK_HEX|TOK_CHAR|TOK_REF)? TOK_MOD* TOK_NEWLINE
void assemble()
{
	while(true)
	{
		tok = yylex();
		if(tok == TOK_EOF) {
			break;
		}
		if(tok == TOK_LABEL)
		{
			int len = strlen(yytext);
			yytext[len - 1] = '\0'; // remove colon
		
			list_insert(&labels, yytext, entryPoint);
			
			tok = yylex();
		}
		if(tok != TOK_NEWLINE)
		{
			instruction_t current = { 0 };
		
			apply_modifiers(&current);
			
			if(tok != TOK_MNEMONIC)
			{
				compile_error("Expected mnemonic, got '%s' instead.", yytext);
				exit(1);
			}
			
			// Gather mnemonic to generate base instruction
			for(int i = 0; ; i++)
			{
				if(mnemonics[i].name == NULL) {
					compile_error("Invalid mnemonic: '%s'", yytext);
					exit(1);
				}
				else if(strcmp(mnemonics[i].name, yytext) == 0) {
					current = mnemonics[i].instr;
					break;
				}
			}
			
			tok = yylex();
			
			apply_modifiers(&current);
			
			if(tok != TOK_NEWLINE)
			{
				switch(tok)
				{
					case TOK_INT:
						current.argument = (uint32_t)atoi(yytext);
						break;
					case TOK_HEX:
						current.argument = (uint32_t)strtol(yytext, NULL, 16);
						break;
					case TOK_CHAR:
						current.argument = (uint32_t)yytext[1];
						break;
					case TOK_REFERENCE:
					{
						// insert patch here for deferred argument modification
						// (yytext + 1) removes the leading @
						list_insert(&patches, yytext + 1, entryPoint);
						break;
					}
					default: 
						compile_error("Expected number, character or label reference, got '%s' instead.", yytext);
						exit(1);
				}
				tok = yylex();
				apply_modifiers(&current);
			}
			if(tok != TOK_NEWLINE && tok != TOK_EOF)
			{
				compile_error("Expected end of line, got '%s instead.", yytext);
				exit(1);
			}
			
			// write command:
			fwrite(&current, sizeof(instruction_t), 1, output);
			
			// Increase command index by one
			entryPoint += 1;
		}
	}
}

int main(int argc, char **argv)
{
	output = stdout;

	int c;
	while ((c = getopt(argc, argv, "o:e:s")) != -1)
	{
		switch (c)
		{
		case 'e':
			entryPoint = atoi(optarg);
			break;
		case 's':
			listSymbols = 1;
			break;
		case 'o':
		{			
			FILE *f = fopen(optarg, "wb");
			if(f == NULL) {
				fprintf(stderr, "%f not found.\n", optarg);
				abort();
			}
			if(output != stdout) {
				fclose(output);
			}
			output = f;
			break;
		}
		case '?':
			if (optopt == 'o')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%02x'.\n", optopt);
			return 1;
		default:
			abort();
		}
	}
	for (int index = optind; index < argc; index++)
	{
		FILE *f = fopen(argv[index], "r");
		if(f == NULL) {
			fprintf(stderr, "%f not found.\n", optarg);
			abort();
		}
		yyrestart (f);
		assemble();
	
		fclose(f);
	}
	
	if(listSymbols)
	{
		FILE *fExports = stdout;
		fprintf(fExports, "SYMBOLS\n");
		for(llist_t *it = labels; it != NULL; it = it->next)
		{
			fprintf(fExports, "\t%s\t%X\n", it->name, it->value);
		}
	}
	
	// printf("Patches:\n");
	for(llist_t *it = patches; it != NULL; it = it->next)
	{
		uint32_t target = list_find(&labels, it->name);
		
		// Seek to the target address
		fseek(output, sizeof(instruction_t) * it->value + 4, SEEK_SET);
		fwrite(&target, 1, sizeof(uint32_t), output);
		
		// printf("%d -> %d (%s)\n", it->value, target, it->name);
	}
	
	list_free(&patches);
	list_free(&labels);
	
	if(output != stdout)
		fclose(output);
	
	return 0;
}

void apply_modifier(instruction_t *i, const char *mod)
{
	// printf("[%s]\n", mod);
	if(strncmp("ci:", mod, 3) == 0)
	{
		i->cmdinfo = atoi(mod + 3);
		return;
	}

	if(strncmp("cmd:", mod, 4) == 0)
	{
		fprintf(stderr, "Command specification %s not supported yet.\n");
		exit(1);
	}
	
	if(strcmp("f:yes", mod) == 0)
	{
		i->flags = 1;
	}
	else if(strcmp("f:no", mod) == 0)
	{
		i->flags = 0;
	}
	else if(strcmp("r:discard", mod) == 0)
	{
		i->output = VM_OUTPUT_DISCARD;
	}
	else if(strcmp("r:push", mod) == 0)
	{
		i->output = VM_OUTPUT_PUSH;
	}
	else if(strcmp("r:jump", mod) == 0)
	{
		i->output = VM_OUTPUT_JUMP;
	}
	else if(strcmp("r:jumpr", mod) == 0)
	{
		i->output = VM_OUTPUT_JUMPR;
	}
	else if(strcmp("i0:zero", mod) == 0)
	{
		i->input0 = VM_INPUT_ZERO;
	}
	else if(strcmp("i0:pop", mod) == 0)
	{
		i->input0 = VM_INPUT_POP;
	}
	else if(strcmp("i0:peek", mod) == 0)
	{
		i->input0 = VM_INPUT_PEEK;
	}
	else if(strcmp("i0:arg", mod) == 0)
	{
		i->input0 = VM_INPUT_ARG;
	}
	else if(strcmp("i1:zero", mod) == 0)
	{
		i->input1 = VM_INPUT_ZERO;
	}
	else if(strcmp("i1:pop", mod) == 0)
	{
		i->input1 = VM_INPUT_POP;
	}
	else if(strcmp("ex(z)=x", mod) == 0)
	{
		i->execZ = VM_EXEC_X;
	}
	else if(strcmp("ex(z)=0", mod) == 0)
	{
		i->execZ = VM_EXEC_0;
	}
	else if(strcmp("ex(z)=1", mod) == 0)
	{
		i->execZ = VM_EXEC_1;
	}
	else if(strcmp("ex(n)=x", mod) == 0)
	{
		i->execN = VM_EXEC_X;
	}
	else if(strcmp("ex(n)=0", mod) == 0)
	{
		i->execN = VM_EXEC_0;
	}
	else if(strcmp("ex(n)=1", mod) == 0)
	{
		i->execN = VM_EXEC_1;
	}
	else
	{
		fprintf(stderr, "Unknown modifier: [%s]\n", mod);
		exit(1);
	}
}