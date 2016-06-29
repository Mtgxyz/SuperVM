
CC = gcc

all: explink expdump

explink: explink.c
	$(CC) -g $^ -o explink

expdump: expdump.c
	$(CC) -g $^ -o expdump

test: exp
	./exp -o test.exp $(ARGS)
	hexdump -C test.exp 
