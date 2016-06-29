
CC = gcc

all: exp

exp: exp.c
	$(CC) -g exp.c -o exp

test: exp
	./exp -o test.exp $(ARGS)
	hexdump -C test.exp 
