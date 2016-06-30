




all: as

as: as.c tokens.c ../mnemonics.c
	gcc -g -o ../bin/$@ $^

tokens.c: tokens.y
	flex -o $@ tokens.y