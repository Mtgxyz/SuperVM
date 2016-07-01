
CC = gcc

all: explink expdump emulator as

explink: explink.c
	$(CC) -g -o bin/$@ $^

expdump: expdump.c mnemonics.c disassembler.c
	$(CC) -g -o bin/$@ $^

emulator: emulator.c vm.c
	$(CC) -g -o bin/$@ $^ -lSDL

as:
	make -C as
	
test: exp
	./exp -o test.exp $(ARGS)
	hexdump -C test.exp 

.PHONY: as