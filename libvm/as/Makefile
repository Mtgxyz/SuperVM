all: as

as: as.c tokens.c ../mnemonics.c ../disassembler.c
	gcc -g -o ../bin/$@ $^

tokens.c: tokens.y
	flex -o $@ --yylineno tokens.y